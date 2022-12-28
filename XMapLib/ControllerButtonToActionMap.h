#pragma once
#include "stdafx.h"
#include <syncstream>
#include <typeindex>

#include "DelayManager.h"
#include "VirtualMap.h"
#include "CallbackRange.h"
#include "ControllerSideDetails.h"
#include "KeyboardTranslator.h"
#include "KeyStateMachineInformational.h"

namespace sds
{
	/**
	 * \brief The <b>map type</b> which means the structure holding some controller to action mapping. For operating on controller button to [action] callback fn maps,
	 * it contains and manages the state machine logic used to activate/de-activate the callbacks. The mapping is SendingElementVK to callback list.
	 * \remarks The hope is that this mapping style will make the "engine" processing it more generic and not totally reliant on processing explicit controller button -> keyboard key mappings.
	 * Hence, the callback function ranges for events occurring such as activation, deactivation, repeat.
	 */
	template<typename KeyStateMachine_t = KeyStateMachineInformational>
	struct ControllerButtonToActionMap
	{
	public:
		using StateAndCallbackPair = std::pair<ControllerButtonStateData::ActionType, CallbackRange>;
		// 3rd and optional property for mappings, a grouping for exclusivity.
		//using GroupingProperty_t = int;
		using InpType = ControllerButtonStateData::ActionType;
	public:
		ControllerButtonData ControllerButton;
		ControllerButtonStateData ControllerButtonState;
		KeyboardButtonData KeyboardButton;
		ControllerToKeyMapData KeymapData;
		KeyStateMachine_t KeyStateMachine;

		std::map<ControllerButtonStateData::ActionType, CallbackRange> MappedActionsArray
		{ {
			{ControllerButtonStateData::ActionType::KEYDOWN, CallbackRange{} },
			{ControllerButtonStateData::ActionType::KEYREPEAT, CallbackRange{} },
			{ControllerButtonStateData::ActionType::KEYUP, CallbackRange{} }
		} };

		// Below, a thread local reference to in-use action maps, necessary for certain behaviors (mostly thumbstick related).
		// Non-owning pointers!
		inline static thread_local std::vector<ControllerButtonToActionMap*> thisBuffer;
	public:
		/**
		 * \brief Destructor, removes 'this' from the thread-local static thisBuffer.
		 */
		~ControllerButtonToActionMap()
		{
			using std::ranges::begin, std::ranges::end, std::ranges::find;
			const auto foundResult = find(begin(thisBuffer), end(thisBuffer), this);
			if (foundResult != end(thisBuffer))
				thisBuffer.erase(foundResult);
		}

		ControllerButtonToActionMap()
		: KeyStateMachine(ControllerButtonState, ControllerButton, KeymapData, IsExclusive(), IsExclusiveNoOvertaking(), IsExclusivityBlocked)
		{
			using std::ranges::begin, std::ranges::end, std::ranges::find;
			// Adding "this" to the thread local this buffer
			const auto foundResult = find(begin(thisBuffer), end(thisBuffer), this);
			if (foundResult != end(thisBuffer))
				thisBuffer.emplace_back(this);

		}
		ControllerButtonToActionMap(const ControllerButtonToActionMap& other) = default;
		ControllerButtonToActionMap(ControllerButtonToActionMap&& other) = default;
		ControllerButtonToActionMap& operator=(const ControllerButtonToActionMap& other) = default;
		ControllerButtonToActionMap& operator=(ControllerButtonToActionMap&& other) = default;
	public:
		[[nodiscard]]
		static
		auto GetThisBuffer() noexcept
		-> const std::vector<ControllerButtonToActionMap*>&
		{
			return thisBuffer;
		}
		[[nodiscard]]
		auto IsExclusive() const noexcept
		{
			constexpr auto ZeroVal = decltype(KeymapData.ExclusivityGrouping){};
			return KeymapData.ExclusivityGrouping != ZeroVal;
		}
		[[nodiscard]]
		auto IsExclusiveNoOvertaking() const noexcept
		{
			constexpr auto ZeroVal = decltype(KeymapData.ExclusivityNoOvertakingGrouping){};
			return KeymapData.ExclusivityNoOvertakingGrouping != ZeroVal;
		}

		/**
		 * \brief If the type is exclusive, (not ExclusiveNoOvertaking) it will return true when pressing this key would be overtaking another key.
		 * <p>AKA <b>IsOvertaking()</b> </p>
		 * \return true if this key would be overtaking another in it's exclusivity grouping
		 */
		[[nodiscard]]
		auto IsExclusivityBlocked() const noexcept -> bool
		{
			using std::ranges::all_of, std::ranges::find, std::ranges::find_if, std::ranges::begin, std::ranges::end;
			// Get copy of range to pointers to all mappings in existence.
			const auto& mapBuffer = GetThisBuffer();
			// Get copy of pointers to all mappings in the same exclusivity grouping as this mapping.
			const auto groupedBuffer = std::ranges::views::transform(mapBuffer, [exGroup = KeymapData.ExclusivityGrouping](const auto& elem)
				{
					return elem->KeymapData.ExclusivityGrouping == exGroup;
				});
			return GetGroupedOvertaken(*this, groupedBuffer);
		}

		/**
		 * \brief Runs the update tasks, resetting the timer and LastAction if necessary.
		 */
		auto KeyUpdate()
		{
			using std::chrono::duration_cast, std::chrono::microseconds;

			auto& cbData = ControllerButtonState;
			const auto mapData = KeymapData;
			const bool DoUpdate = (cbData.LastAction == InpType::KEYUP && cbData.LastSentTime.IsElapsed()) && mapData.UsesRepeat;
			const bool DoImmediate = cbData.LastAction == InpType::KEYUP && !mapData.UsesRepeat;
			if (DoUpdate || DoImmediate)
			{
				cbData.LastAction = InpType::NONE;
				cbData.LastSentTime.Reset(duration_cast<microseconds>(KeymapData.DelayAfterRepeatActivation).count());
			}
		}
	public:
		/// <summary>
		/// Operator<< overload for std::ostream specialization,
		///	writes more detailed map details for debugging.
		///	Thread-safe, provided all writes to the ostream object
		///	are wrapped with std::osyncstream!
		/// </summary>
		friend std::ostream& operator<<(std::ostream& os, const ControllerButtonToActionMap& obj)
		{
			const std::type_index ti = typeid(int);
			const bool areSameHash = obj.KeyboardButton.VK.type().hash_code() == ti.hash_code();
			const bool isTypeInt = obj.KeyboardButton.VK.has_value() && areSameHash;
			const int keyboardVK = isTypeInt ? std::any_cast<int>(obj.KeyboardButton.VK) : 0;

			const char printed = Utilities::VirtualMap::GetCharFromVK(keyboardVK);
			const bool isPrintable = std::isprint(static_cast<unsigned char>(printed));
			std::osyncstream ss(os);
			ss << "[ControllerButtonToActionMap]" << " ";
			ss << "SendingElementVK:" << obj.ControllerButton.VK << " ";
			ss << "MappedToVK:" << keyboardVK << " ";
			ss << "MappedToVK(AKA):" << (isPrintable ? printed : ' ') << " ";
			ss << "UsesRepeat:" << obj.KeymapData.UsesRepeat << " ";
			ss << obj.ControllerButtonState << " ";
			ss << "[/ControllerButtonToActionMap]" << " ";
			return os;
		}
		/// <summary>
		/// Operator<< overload for std::string specialization,
		///	writes relevant map details to the std::string.
		///	NOTE: Changing this will break the DLL API which depends on this format.
		/// </summary>
		//TODO fix this
		//friend std::string& operator<<(std::string& os, const ControllerButtonToActionMap& obj)
		//{
		//	const char printed = Utilities::VirtualMap::GetCharFromVK(obj.KeyboardButton.VK);
		//	const bool isPrintable = std::isprint(static_cast<unsigned char>(printed));
		//	std::stringstream ss;
		//	ss << "[ControllerButtonToActionMap]" << " ";
		//	ss << "SendingElementVK:" << obj.ControllerButton.VK << " ";
		//	ss << "MappedToVK:" << obj.KeyboardButton.VK << " ";
		//	ss << "MappedToVK(AKA):" << (isPrintable? printed : ' ') << " ";
		//	ss << "UsesRepeat:" << std::boolalpha << obj.KeymapData.UsesRepeat << " ";
		//	ss << "[/ControllerButtonToActionMap]" << " ";
		//	os += ss.str();
		//	return os;
		//}
		friend bool operator==(const ControllerButtonToActionMap& lhs, const ControllerButtonToActionMap& rhs) noexcept
		{
			const std::type_index ti = typeid(int);
			const bool lhsSameHash = lhs.KeyboardButton.VK.type().hash_code() == ti.hash_code();
			const bool rhsSameHash = rhs.KeyboardButton.VK.type().hash_code() == ti.hash_code();
			const bool isLhsTypeInt = lhs.KeyboardButton.VK.has_value() && lhsSameHash;
			const bool isRhsTypeInt = rhs.KeyboardButton.VK.has_value() && rhsSameHash;
			const int lhsKeyboardVK = isLhsTypeInt ? std::any_cast<int>(lhs.KeyboardButton.VK) : 0;
			const int rhsKeyboardVK = isRhsTypeInt ? std::any_cast<int>(rhs.KeyboardButton.VK) : 0;
			//TODO silently ignoring the possible error state here where one is type int and one isn't

			return lhs.ControllerButton.VK == rhs.ControllerButton.VK
				&& lhsKeyboardVK == rhsKeyboardVK;
		}
		friend bool operator!=(const ControllerButtonToActionMap& lhs, const ControllerButtonToActionMap& rhs) noexcept
		{
			return !(lhs == rhs);
		}
	private:
		/**
		 * \brief Checks the exclusivity group members for a key being overtaken by the newly pressed key.
		 * \param detail button newly pressed
		 * \param groupedBuffer buffer holding the state of all keys at present within the grouping
		 * \return optional, pointer to cbtam being overtaken
		 */
		auto GetGroupedOvertaken(
			const ControllerButtonToActionMap& detail,
			const auto& groupedBuffer) const noexcept -> bool
		{
			using std::ranges::find_if, std::ranges::end;
			// If exclusivity with update grouping has some other members...
			if (!groupedBuffer.empty())
			{
				auto IsGroupedBtnPressedFn = [&](const ControllerButtonToActionMap* groupedButtonElem)
				{
					const auto elemState = groupedButtonElem->ControllerButtonState.LastAction;
					const auto elemButtonVK = groupedButtonElem->ControllerButton.VK;
					constexpr auto DownType = InpType::KEYDOWN;
					constexpr auto RepeatType = InpType::KEYREPEAT;
					const bool isOtherButtonPressed = elemState == DownType || elemState == RepeatType;
					// this is the case where the key we're testing for is the same key we're investigating
					const bool isSameKey = elemButtonVK == detail.ControllerButton.VK;
					if (!isSameKey)
					{
						return isOtherButtonPressed;
					}
					return false;
				};

				const auto mpit = find_if(groupedBuffer, IsGroupedBtnPressedFn);
				if (mpit == end(groupedBuffer))
				{
					return false;
				}
				return true;
			}
			return false;
		}
	};
}

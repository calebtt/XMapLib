#pragma once
#include "stdafx.h"
#include <syncstream>
#include <typeindex>

#include "DelayManager.h"
#include "VirtualMap.h"
#include "CallbackRange.h"
#include "ControllerSideDetails.h"
#include "KeyboardTranslator.h"

namespace sds
{
	/// <summary> The <b>map type</b> which means the structure holding some controller to action mapping.
	/// For operating on controller button to [action] callback fn maps,
	///	it contains and manages the state machine logic used to activate/de-activate the callbacks.
	/// The mapping is SendingElementVK to callback list. </summary>
	///	<remarks> The hope is that this mapping style will make the "engine" processing it more
	///	generic and not totally reliant on processing explicit controller button -> keyboard key mappings.
	///	Hence, the callback function ranges for events occurring such as activation, deactivation, repeat. </remarks>
	struct ControllerButtonToActionMap
	{
	public:
		//TODO this class should be the one constructing a default mapping.
		//So, we will template it for a concept interface that has things like "Normal()" etc.
		//using MappingFuncs = KeyboardTranslator;
		using ClockType = std::chrono::high_resolution_clock;
		using PointInTime = std::chrono::time_point<ClockType>;
		using StateAndCallbackPair = std::pair<ControllerButtonStateData::ActionType, CallbackRange>;
		// 3rd and optional property for mappings, a grouping for exclusivity.
		using GroupingProperty_t = int;
	public:
		ControllerButtonData ControllerButton;
		ControllerButtonStateData ControllerButtonState;
		//TODO probably remove this, the data will be stored in the type-erasure of the std::function holding the lambda fn being called.
		KeyboardButtonData KeyboardButton;
		//ActionRanges MappedActions;
		ControllerToKeyMapData KeymapData;
		GroupingProperty_t ExclusivityGrouping{};

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

		/// <summary>
		/// Destructor, removes 'this' from the thread-local static thisBuffer.
		/// </summary>
		~ControllerButtonToActionMap()
		{
			using std::ranges::begin, std::ranges::end, std::ranges::find;
			const auto foundResult = find(begin(thisBuffer), end(thisBuffer), this);
			if (foundResult != end(thisBuffer))
				thisBuffer.erase(foundResult);
		}

		ControllerButtonToActionMap()
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
		static auto GetThisBuffer() noexcept
		{
			return thisBuffer;
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
	};
}

#pragma once
#include "stdafx.h"
#include <syncstream>
#include <typeindex>

#include "DelayManager.h"
#include "VirtualMap.h"
#include "CallbackRange.h"
#include "ControllerSideDetails.h"

namespace sds
{
	/// <summary> For operating on controller button to [action] callback fn maps,
	///	it contains and manages the state machine logic used to activate/de-activate the callbacks.
	/// The mapping is SendingElementVK to callback list. </summary>
	///	<remarks> The hope is that this mapping style will make the "engine" processing it more
	///	generic and not totally reliant on processing explicit controller button -> keyboard key mappings.
	///	Hence, the callback function ranges for events occurring such as activation, deactivation, repeat. </remarks>
	struct ControllerButtonToActionMap
	{
		using ClockType = std::chrono::high_resolution_clock;
		using PointInTime = std::chrono::time_point<ClockType>;
		struct ActionRanges
		{
			// Tasks called on key-down of the controller button (key-down event)
			CallbackRange ActivationTasks;
			// Tasks called on key-up of the controller button (key-up event)
			CallbackRange DeactivationTasks;
			// Tasks called on key-repeat of the controller button (key-repeat event)
			CallbackRange RepeatTasks;
			/// <summary> These are the tasks called on destruction (or otherwise, cleanup), but not by this instance. </summary>
			///	<remarks> Not called on destruction of this instance, because that would imply mutating non-local state.
			///	They *may* be called by a state machine or container managing a collection. It would be a surprise for a user
			///	to copy an instance of this class and upon destruction notice that it sent a key-up event or similar. </remarks>
			CallbackRange CleanupTasks;
		};
	public:
		ControllerButtonData ControllerButton;
		ControllerButtonStateData ControllerButtonState;
		KeyboardButtonData KeyboardButton;
		ActionRanges MappedActions;
		ControllerToKeyMapData KeymapData;
	public:
		//Ctor
		ControllerButtonToActionMap(
			const int controllerElementVK, 
			const bool useRepeat)
				:
		ControllerButton{controllerElementVK},
		//KeyboardButton{keyboardMouseElementVK},
		KeymapData{useRepeat}
		{
			
		}
		ControllerButtonToActionMap() = default;
		ControllerButtonToActionMap(const ControllerButtonToActionMap& other) = default;
		ControllerButtonToActionMap(ControllerButtonToActionMap&& other) = default;
		ControllerButtonToActionMap& operator=(const ControllerButtonToActionMap& other) = default;
		ControllerButtonToActionMap& operator=(ControllerButtonToActionMap&& other) = default;
		~ControllerButtonToActionMap() = default;
		/// <summary>
		/// Operator<< overload for std::ostream specialization,
		///	writes enum class:int value as decimal int value.
		///	Thread-safe, provided all writes to the ostream object
		///	are wrapped with std::osyncstream!
		/// </summary>
		friend std::ostream& operator<<(std::ostream& os, const ControllerButtonStateData::ActionType& obj)
		{
			std::osyncstream ss(os);
			ss << static_cast<std::underlying_type_t<ControllerButtonStateData::ActionType>>(obj);
			return os;
		}
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
			ss << "LastAction:" << obj.ControllerButtonState.LastAction << " ";
			ss << obj.ControllerButtonState.LastSentTime << " ";
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

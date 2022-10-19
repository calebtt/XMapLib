#pragma once
#include "stdafx.h"
#include "DelayManager.h"
#include "VirtualMap.h"
#include "CallbackRange.h"
#include <syncstream>

namespace sds
{
	/// <summary>
	/// Somewhat generic utility class for holding controller button to keyboard button maps.
	/// </summary>
	struct ControllerButtonToActionMap
	{
		using ClockType = std::chrono::high_resolution_clock;
		using PointInTime = std::chrono::time_point<ClockType>;
		enum class ActionType : int
		{
			NONE = 0,
			KEYDOWN = XINPUT_KEYSTROKE_KEYDOWN,
			KEYREPEAT = XINPUT_KEYSTROKE_REPEAT,
			KEYUP = XINPUT_KEYSTROKE_KEYUP
		};
	public:
		//Struct members
		int SendingElementVK{ 0 }; // VK of controller button
		int MappedToVK{ 0 }; // VK of mapped-to input (key or mouse button)
		bool UsesRepeat{ true }; // Uses the key-repeat behavior when held down
		ActionType LastAction{ ActionType::NONE };
		Utilities::DelayManager LastSentTime{ KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT };
		// Functions/tasks to be called on activation.
		CallbackRange ActivationTasks;
		CallbackRange DeactivationTasks;
		CallbackRange RepeatTasks;
	public:
		//Ctor
		ControllerButtonToActionMap(const int controllerElementVK, const int keyboardMouseElementVK, const bool useRepeat)
			: SendingElementVK(controllerElementVK), MappedToVK(keyboardMouseElementVK), UsesRepeat(useRepeat)
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
		friend std::ostream& operator<<(std::ostream& os, const ActionType& obj)
		{
			std::osyncstream ss(os);
			ss << static_cast<int>(obj);
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
			const char printed = Utilities::VirtualMap::GetCharFromVK(obj.MappedToVK);
			const bool isPrintable = std::isprint(static_cast<unsigned char>(printed));
			std::osyncstream ss(os);
			ss << "[ControllerButtonToActionMap]" << " ";
			ss << "SendingElementVK:" << obj.SendingElementVK << " ";
			ss << "MappedToVK:" << obj.MappedToVK << " ";
			ss << "MappedToVK(AKA):" << (isPrintable ? printed : ' ') << " ";
			ss << "UsesRepeat:" << obj.UsesRepeat << " ";
			ss << "LastAction:" << obj.LastAction << " ";
			ss << obj.LastSentTime << " ";
			ss << "[/ControllerButtonToActionMap]" << " ";
			return os;
		}
		/// <summary>
		/// Operator<< overload for std::string specialization,
		///	writes relevant map details to the std::string.
		///	NOTE: Changing this will break the DLL API which depends on this format.
		/// </summary>
		friend std::string& operator<<(std::string& os, const ControllerButtonToActionMap& obj)
		{
			const char printed = Utilities::VirtualMap::GetCharFromVK(obj.MappedToVK);
			const bool isPrintable = std::isprint(static_cast<unsigned char>(printed));
			std::stringstream ss;
			ss << "[ControllerButtonToActionMap]" << " ";
			ss << "SendingElementVK:" << obj.SendingElementVK << " ";
			ss << "MappedToVK:" << obj.MappedToVK << " ";
			ss << "MappedToVK(AKA):" << (isPrintable? printed : ' ') << " ";
			ss << "UsesRepeat:" << std::boolalpha << obj.UsesRepeat << " ";
			ss << "[/ControllerButtonToActionMap]" << " ";
			os += ss.str();
			return os;
		}
		friend bool operator==(const ControllerButtonToActionMap& lhs, const ControllerButtonToActionMap& rhs)
		{
			return lhs.SendingElementVK == rhs.SendingElementVK
				&& lhs.MappedToVK == rhs.MappedToVK;
		}
		friend bool operator!=(const ControllerButtonToActionMap& lhs, const ControllerButtonToActionMap& rhs)
		{
			return !(lhs == rhs);
		}
	};
}

#pragma once
#include "stdafx.h"
#include "DelayManager.h"
#include "VirtualMap.h"
#include "CallbackRange.h"
#include <syncstream>

namespace sds
{
	/// <summary>
	/// The specific data used to describe the controller button in the mapping.
	/// </summary>
	struct ControllerButtonData
	{
		// VK of controller button
		int VK{ 0 };
		//int SendingElementVK{ 0 };
	};

	/// <summary>
	/// The specific data used to describe the mapped-to (kbd/mouse) key in the mapping.
	/// </summary>
	struct MappedKeyData
	{
		// VK of mapped-to input (key or mouse button)
		int VK{ 0 };
		//int MappedToVK{ 0 };
	};

	/// <summary>
	/// The extra information regarding the controller button to keyboard key mapping.
	/// </summary>
	struct ControllerToKeyMapData
	{
		// Uses the key-repeat behavior when held down
		bool UsesRepeat{ true };
	};

	/// <summary>
	/// Specific info regarding the state machine which is used to track the events being fired,
	///	based on what the controller button reports.
	/// </summary>
	struct ControllerButtonStateData
	{
		enum class ActionType : int
		{
			NONE = 0,
			KEYDOWN = XINPUT_KEYSTROKE_KEYDOWN,
			KEYREPEAT = XINPUT_KEYSTROKE_REPEAT,
			KEYUP = XINPUT_KEYSTROKE_KEYUP
		};
		// state machine info for controller btn
		ActionType LastAction{ ActionType::NONE };
		// last sent time, normally used for key repeat
		Utilities::DelayManager LastSentTime{ KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT };
	};


	/// <summary> For operating on controller button to [action] callback fn maps,
	///	it contains and manages the state machine logic used to activate/de-activate the callbacks.
	/// The mapping is SendingElementVK to callback list. </summary>
	template<typename Activation_t = CallbackRange,
	typename Deactivation_t = CallbackRange,
	typename Repeat_t = CallbackRange,
	typename Cleanup_t = CallbackRange>
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
		// VK of controller button
		int SendingElementVK{ 0 };
		// VK of mapped-to input (key or mouse button)
		int MappedToVK{ 0 };
		// Uses the key-repeat behavior when held down
		bool UsesRepeat{ true }; 
		ActionType LastAction{ ActionType::NONE };
		Utilities::DelayManager LastSentTime{ KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT };

		// Tasks called on key-down of the controller button (key-down event)
		Activation_t ActivationTasks;
		// Tasks called on key-up of the controller button (key-up event)
		Deactivation_t DeactivationTasks;
		// Tasks called on key-repeat of the controller button (key-repeat event)
		Repeat_t RepeatTasks;
		/// <summary> These are the tasks called on destruction (or otherwise, cleanup), but not by this instance. </summary>
		///	<remarks> Not called on destruction of this instance, because that would imply mutating non-local state.
		///	They *may* be called by a state machine or container managing a collection. It would be a surprise for a user
		///	to copy an instance of this class and upon destruction notice that it sent a key-up event or similar. </remarks>
		Cleanup_t CleanupTasks;
	public:
		//Ctor
		ControllerButtonToActionMap(
			const int controllerElementVK, 
			const int keyboardMouseElementVK, 
			const bool useRepeat)
			:
		SendingElementVK(controllerElementVK),
		MappedToVK(keyboardMouseElementVK),
		UsesRepeat(useRepeat)
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
			ss << static_cast<std::underlying_type<ActionType>>(obj);
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

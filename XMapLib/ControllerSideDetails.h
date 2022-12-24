#pragma once
#include "stdafx.h"
#include <any>
#include <ostream>

// Most of these structs are used by ControllerButtonToActionMap
namespace sds
{
	/// <summary>
	/// The specific data used to describe the controller button in the mapping.
	///	This type is probably relatively concrete within the mapping logic.
	/// </summary>
	struct ControllerButtonData
	{
		// VK of controller button
		int VK{ 0 };

		friend bool operator==(const ControllerButtonData& lhs, const ControllerButtonData& rhs)
		{
			return lhs.VK == rhs.VK;
		}

		friend bool operator!=(const ControllerButtonData& lhs, const ControllerButtonData& rhs)
		{
			return !(lhs == rhs);
		}
	};

	/// <summary>
	/// The specific data used to describe the mapped-to (kbd/mouse) key in the mapping.
	/// </summary>
	struct KeyboardButtonData
	{
		// Presumably, the VK of mapped-to input (key or mouse button)
		// but if some other data is stored here, that is fine too.
		std::any VK;
	};

	/// <summary>
	/// The extra information regarding the controller button to keyboard key mapping.
	/// </summary>
	struct ControllerToKeyMapData
	{
		using cms = std::chrono::milliseconds;
		// Uses the key-repeat behavior when held down
		bool UsesRepeat{ true };
		// Delay before activating the key-repeat behavior.
		cms DelayBeforeRepeatActivation{ cms{250} };
		// Delay after or in-between key repeat events.
		cms DelayAfterRepeatActivation{ cms{250} };
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
		//struct IdleState {
		//	ControllerButtonStateData::ActionType value = ControllerButtonStateData::ActionType
		//		::NONE;
		//};
		//struct KeyDownState {
		//	ControllerButtonStateData::ActionType value = ControllerButtonStateData::ActionType
		//		::KEYDOWN;
		//};
		//struct KeyRepeatState {
		//	ControllerButtonStateData::ActionType value = ControllerButtonStateData::ActionType
		//		::KEYREPEAT;
		//};
		//struct KeyUpState {
		//	ControllerButtonStateData::ActionType value = ControllerButtonStateData::ActionType
		//		::KEYUP;
		//};
		//std::variant<IdleState, KeyDownState, KeyUpState, KeyRepeatState> LastAction = IdleState{};

		// state machine info for controller btn
		ActionType LastAction{ ActionType::NONE };
		// last sent time, normally used for key repeat
		Utilities::DelayManager LastSentTime{ KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT };

		friend std::ostream& operator<<(std::ostream& os, const ControllerButtonStateData& obj)
		{
			constexpr bool isInt = std::is_same_v<std::underlying_type<ActionType>, int>;
			if constexpr(isInt)
			{
				return os
					<< "LastAction: " << static_cast<int>(obj.LastAction)
					<< " LastSentTime: " << obj.LastSentTime;
			}
			return os << "ERROR";
		}
	};
}

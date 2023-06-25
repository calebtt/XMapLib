#pragma once
#include "CustomTypes.h"

namespace sds
{
	/**
	 * \brief A data structure to hold player information. A default constructed
	 * KeyboardPlayerInfo struct has default values that are usable. 
	 */
	struct KeyboardPlayerInfo
	{
		int player_id{ 0 };
	};

	/**
	 * \brief Some constants that will someday be configurable. If these are used with config file loaded values,
	 * this type should do that upon construction, with a non-configurable XML file name.
	 */
	struct KeyboardSettings
	{
		/**
		 * \brief Delay each iteration of a polling loop, short enough to not miss information, long enough to not waste CPU cycles.
		 */
		static constexpr detail::NanosDelay_t PollingLoopDelay{ std::chrono::milliseconds{1} };
		/**
		 * \brief Key Repeat Delay is the time delay a button has in-between activations.
		 */
		static constexpr detail::NanosDelay_t KeyRepeatDelay{ std::chrono::microseconds{100'000} };
		/**
		 * \brief The button virtual keycodes as a flat array.
		 */
		static constexpr std::array<detail::VirtualKey_t, 14> ButtonCodeArray
		{
			XINPUT_GAMEPAD_DPAD_UP,
			XINPUT_GAMEPAD_DPAD_DOWN,
			XINPUT_GAMEPAD_DPAD_LEFT,
			XINPUT_GAMEPAD_DPAD_RIGHT,
			XINPUT_GAMEPAD_START,
			XINPUT_GAMEPAD_BACK,
			XINPUT_GAMEPAD_LEFT_THUMB,
			XINPUT_GAMEPAD_RIGHT_THUMB,
			XINPUT_GAMEPAD_LEFT_SHOULDER,
			XINPUT_GAMEPAD_RIGHT_SHOULDER,
			XINPUT_GAMEPAD_A,
			XINPUT_GAMEPAD_B,
			XINPUT_GAMEPAD_X,
			XINPUT_GAMEPAD_Y
		};

		// used internally to denote left or right triggers, similar to the button VKs though they may
		// not be used by the OS API state updates in the same way--we virtualize them.
		static constexpr detail::VirtualKey_t LeftTriggerVk{VK_GAMEPAD_LEFT_TRIGGER};
		static constexpr detail::VirtualKey_t RightTriggerVk{VK_GAMEPAD_RIGHT_TRIGGER};

		static constexpr detail::ThumbstickValue_t LeftStickDeadzone{XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE};
		static constexpr detail::ThumbstickValue_t RightStickDeadzone{XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE};

		static constexpr detail::TriggerValue_t LeftTriggerThreshold{XINPUT_GAMEPAD_TRIGGER_THRESHOLD};
		static constexpr detail::TriggerValue_t RightTriggerThreshold{XINPUT_GAMEPAD_TRIGGER_THRESHOLD};
		// The type of the button buffer without const/volatile/reference.
		using ButtonBuffer_t = std::remove_reference_t< std::remove_cv_t<decltype(ButtonCodeArray)> >;
	};

	/**
	 * \brief For no other reason but to make the common task of injecting these down the architecture less verbose.
	 */
	struct KeyboardSettingsPack
	{
		KeyboardPlayerInfo playerInfo;
		KeyboardSettings settings;
	};
}
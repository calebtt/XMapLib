#pragma once
#include "stdafx.h"
namespace sds
{
	/// <summary> Some mouse movement mapping program specific constants that might someday be configurable,
	/// and the functions that help validate associated values. </summary>
	struct MouseSettings
	{
		//Pixels Magnitude is the number of pixels each iteration of the loop sends in MouseMoveThread.
		static constexpr int PIXELS_MAGNITUDE{ 1 };
		//Pixels Nomove is the number of pixels each iteration of the loop sends in MouseMoveThread to indicate no movement.
		static constexpr int PIXELS_NOMOVE{ 0 };
		//Input Poller thread delay, in milliseconds.
		static constexpr int THREAD_DELAY_INPUT_POLLER_MS{ 4 };
		//ThumbstickValueMax is the sentinel abs value a controller thumbstick should report.
		static constexpr int ThumbstickValueMax{ 32'766 };
		//ThumbstickValueMin is the sentinel min value a controller thumbstick should report.
		static constexpr int ThumbstickValueMin{ -ThumbstickValueMax };
		//PolarRadiusValueMax is the sentinel abs value of a computed polar radius.
		static constexpr int PolarRadiusValueMax{ 39'000 };
		//Sensitivity Min is the minimum mouse sensitivity value allowed, used in several places.
		static constexpr int SENSITIVITY_MIN{ 1 };
		//Sensitivity Max is the maximum mouse sensitivity value allowed, used in several places.
		static constexpr int SENSITIVITY_MAX{ 100 };
		//Sensitivity Default is the default sensitivity value if none is given during construction of the mouse object.
		static constexpr int SENSITIVITY_DEFAULT{ 35 };
		//Microseconds Min is the minimum delay for the thumbstick axis thread loop at the highest thumbstick value.
		static constexpr int MICROSECONDS_MIN{ 1500 };
		//Microseconds Max is the maximum delay for the thumbstick axis thread loop at the lowest thumbstick value.
		static constexpr int MICROSECONDS_MAX{ 32'000 };
		//Deadzone Min is the minimum allowable value for a thumbstick deadzone.
		static constexpr int DEADZONE_MIN{ 1 };
		//Deadzone Max is the maximum allowable value for a thumbstick deadzone.
		static constexpr int DEADZONE_MAX{ std::numeric_limits<SHORT>::max() - 1 };
		//Deadzone Default is the default deadzone value for a thumbstick.
		static constexpr int DEADZONE_DEFAULT{ XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE };
		//Alt Deadzone Default is the multiplier to use when a deadzone is already activated,
		//the deadzone value for the other axis is lessened via this value.
		static constexpr float ALT_DEADZONE_MULT_DEFAULT{ 0.45f };
		//File name for the file containing the thumbstick scaling values.
		static constexpr std::string_view SCALING_VALUES_FNAME{ "adjustment_values.txt" };
		//Static assertions about the const members
		static_assert(SENSITIVITY_MAX < MICROSECONDS_MAX);
		static_assert(SENSITIVITY_MIN >= 1);
		static_assert(SENSITIVITY_MIN < SENSITIVITY_MAX);
		static_assert(DEADZONE_MIN > 0);
		static_assert(DEADZONE_MAX < std::numeric_limits<SHORT>::max());
		static_assert(MICROSECONDS_MIN < MICROSECONDS_MAX);
		[[nodiscard]] static constexpr bool IsValidSensitivityValue(const int newSens) noexcept
		{
			return (newSens <= SENSITIVITY_MAX) && (newSens >= SENSITIVITY_MIN);
		}
		[[nodiscard]] static constexpr bool IsValidDeadzoneValue(const int dz) noexcept
		{
			return (dz <= DEADZONE_MAX) && (dz >= DEADZONE_MIN);
		}
	};
	/// <summary> Used as a data package to hold player information, includes thumbstick deadzone information.</summary>
	/// <remarks> A default constructed MousePlayerInfo struct has default values that are usable. </remarks> 
	struct MousePlayerInfo
	{
		using PidType = int;
		using DzType = int;
		//ISO CPP guidelines C.45 followed here: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-default
		std::atomic<DzType> left_polar_dz{ XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE }; // left stick polar radius dz
		std::atomic<DzType> right_polar_dz{ XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE }; // right stick polar radius dz
		std::atomic<PidType> player_id{ 0 };
		//default ctor
		MousePlayerInfo() = default;
		//copy constructor
		MousePlayerInfo(const MousePlayerInfo& sp)
		{
			left_polar_dz.exchange(sp.left_polar_dz);
			right_polar_dz.exchange(sp.right_polar_dz);
			player_id.exchange(sp.player_id);
		}
		//assignment
		MousePlayerInfo& operator=(const MousePlayerInfo& sp)
		{
			if (this == &sp)
				return *this;
			left_polar_dz.exchange(sp.left_polar_dz);
			right_polar_dz.exchange(sp.right_polar_dz);
			player_id.exchange(sp.player_id);
			return *this;
		}
		//move constructor
		MousePlayerInfo(MousePlayerInfo&& sp) = delete;
		//Move assignment operator
		MousePlayerInfo& operator=(MousePlayerInfo&& sp) = delete;
		~MousePlayerInfo() = default;
	};

	/// <summary>For no other reason but to make the common task of passing these down the architecture less verbose. </summary>
	struct MouseSettingsPack
	{
		MousePlayerInfo playerInfo;
		MouseSettings settings;
	};
}
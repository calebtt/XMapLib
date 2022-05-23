#pragma once
namespace sds
{
	/// <summary>
	/// Some constants that might someday be configurable, and the functions that help validate associated values.
	/// </summary>
	struct MouseSettings
	{
		//Pixels Magnitude is the number of pixels each iteration of the loop
		//sends in ThumbstickAxisThread
		static constexpr int PIXELS_MAGNITUDE{ 1 };
		//Pixels Nomove is the number of pixels each iteration of the loop
		//sends in ThumbstickAxisThread for the opposite axis the thread is concerned with.
		static constexpr int PIXELS_NOMOVE{ 0 };
		//Input Poller thread delay, in milliseconds.
		static constexpr int THREAD_DELAY_INPUT_POLLER_MS{ 4 };
		//ThumbstickValueMax is the sentinel abs value a controller thumbstick should report.
		static constexpr short ThumbstickValueMax{ 30'000 };
		//ThumbstickValueMin is the sentinel min value a controller thumbstick should report.
		static constexpr short ThumbstickValueMin{ -ThumbstickValueMax };
		//Sensitivity Min is the minimum mouse sensitivity value allowed, used in several places.
		static constexpr int SENSITIVITY_MIN{ 1 };
		//Sensitivity Max is the maximum mouse sensitivity value allowed, used in several places.
		static constexpr int SENSITIVITY_MAX{ 100 };
		//Sensitivity Default is the default sensitivity value if none is given.
		static constexpr int SENSITIVITY_DEFAULT{ 35 };
		//Microseconds Min is the minimum delay for the thumbstick axis thread loop at the highest thumbstick value.
		static constexpr int MICROSECONDS_MIN{ 1100 };
		//Microseconds Max is the maximum delay for the thumbstick axis thread loop at the lowest thumbstick value.
		static constexpr int MICROSECONDS_MAX{ 24'000 };
		//Deadzone Min is the minimum allowable value for a thumbstick deadzone.
		static constexpr int DEADZONE_MIN{ 1 };
		//Deadzone Max is the maximum allowable value for a thumbstick deadzone.
		static constexpr int DEADZONE_MAX{ std::numeric_limits<SHORT>::max() - 1 };
		//Deadzone Default is the default deadzone value for a thumbstick.
		static constexpr int DEADZONE_DEFAULT{ XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE };
		//Alt Deadzone Default is the multiplier to use when a deadzone is already activated,
		//the deadzone value for the other axis is lessened via this value.
		static constexpr float ALT_DEADZONE_MULT_DEFAULT{ 0.5f };
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
}


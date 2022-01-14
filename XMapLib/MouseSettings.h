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
		constexpr static const int PIXELS_MAGNITUDE = 1;
		//Pixels Nomove is the number of pixels each iteration of the loop
		//sends in ThumbstickAxisThread for the opposite axis the thread is concerned with.
		constexpr static const int PIXELS_NOMOVE = 0;
		//Input Poller thread delay, in milliseconds.
		constexpr static const int THREAD_DELAY_POLLER = 10;
		//SMax is the value of the Microsoft type "SHORT"'s maximum possible value.
		constexpr static const short SMax = std::numeric_limits<SHORT>::max();
		//SMin is the value of the Microsoft type "SHORT"'s minimum possible value.
		constexpr static const short SMin = std::numeric_limits<SHORT>::min();
		//Sensitivity Min is the minimum mouse sensitivity value allowed, used in several places.
		constexpr static const int SENSITIVITY_MIN = 1;
		//Sensitivity Max is the maximum mouse sensitivity value allowed, used in several places.
		constexpr static const int SENSITIVITY_MAX = 100;
		//Sensitivity Default is the default sensitivity value if none is given.
		constexpr static const int SENSITIVITY_DEFAULT = 35;
		//Microseconds Min is the minimum delay for the thumbstick axis thread loop at the highest thumbstick value.
		constexpr static const int MICROSECONDS_MIN = 500;
		//Microseconds Max is the maximum delay for the thumbstick axis thread loop at the lowest thumbstick value.
		constexpr static const int MICROSECONDS_MAX = 18000;
		//Microseconds Min Max is the minimum delay's maximum value for the thumbstick axis thread loop at the lowest sensitivity value.
		constexpr static const int MICROSECONDS_MIN_MAX = MICROSECONDS_MIN * 3;
		//Deadzone Min is the minimum allowable value for a thumbstick deadzone.
		constexpr static const int DEADZONE_MIN = 1;
		//Deadzone Max is the maximum allowable value for a thumbstick deadzone.
		constexpr static const int DEADZONE_MAX = std::numeric_limits<SHORT>::max() - 1;
		//Deadzone Default is the default deadzone value for a thumbstick.
		constexpr static const int DEADZONE_DEFAULT = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		//Alt Deadzone Default is the multiplier to use when a deadzone is already activated,
		//the deadzone value for the other axis is lessened via this value.
		constexpr static const float ALT_DEADZONE_MULT_DEFAULT = 0.75f;
		//Static assertions about the const members
		static_assert(SENSITIVITY_MAX < MICROSECONDS_MAX);
		static_assert(SENSITIVITY_MIN >= 1);
		static_assert(SENSITIVITY_MIN < SENSITIVITY_MAX);
		static_assert(DEADZONE_MIN > 0);
		static_assert(DEADZONE_MAX < std::numeric_limits<SHORT>::max());
		static_assert(MICROSECONDS_MIN < MICROSECONDS_MAX);
		static_assert(MICROSECONDS_MIN_MAX < MICROSECONDS_MAX);
		static_assert(MICROSECONDS_MIN_MAX > MICROSECONDS_MIN);
		static constexpr bool IsValidSensitivityValue(int newSens) noexcept
		{
			return (newSens <= SENSITIVITY_MAX) && (newSens >= SENSITIVITY_MIN);
		}
		static constexpr bool IsValidDeadzoneValue(int dz) noexcept
		{
			return (dz <= DEADZONE_MAX) && (dz >= DEADZONE_MIN);
		}
		static constexpr bool IsValidThumbstickValue(int thumb) noexcept
		{
			return (thumb <= SMax) && (thumb >= SMin);
		}
	};
}


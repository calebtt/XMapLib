#pragma once
namespace sds
{
	/// <summary>
	/// Some constants that might someday be configurable.
	/// </summary>
	struct KeyboardSettings
	{
		//Input Poller maximum number of XINPUT_KEYSTROKE structs to queue before dropping input.
		constexpr static const int MAX_STATE_COUNT = 128;
		//Input Poller thread delay, in milliseconds.
		constexpr static const int THREAD_DELAY_POLLER = 10;
		//Microseconds Delay Keyrepeat is the time delay a button has in between activations.
		constexpr static const int MICROSECONDS_DELAY_KEYREPEAT = 100000;
		//It is necessary to be able to distinguish these mapping values in KeyboardTranslator.
		constexpr static std::array<int,8> THUMBSTICK_L_VK_LIST
		{
			VK_PAD_LTHUMB_UP,
			VK_PAD_LTHUMB_DOWN,
			VK_PAD_LTHUMB_RIGHT,
			VK_PAD_LTHUMB_LEFT,
			VK_PAD_LTHUMB_UPLEFT,
			VK_PAD_LTHUMB_UPRIGHT,
			VK_PAD_LTHUMB_DOWNRIGHT,
			VK_PAD_LTHUMB_DOWNLEFT
		};
		constexpr static std::array<int, 8> THUMBSTICK_R_VK_LIST
		{
			VK_PAD_RTHUMB_UP,
			VK_PAD_RTHUMB_DOWN,
			VK_PAD_RTHUMB_RIGHT,
			VK_PAD_RTHUMB_LEFT,
			VK_PAD_RTHUMB_UPLEFT,
			VK_PAD_RTHUMB_UPRIGHT,
			VK_PAD_RTHUMB_DOWNRIGHT,
			VK_PAD_RTHUMB_DOWNLEFT
		};
	};
}


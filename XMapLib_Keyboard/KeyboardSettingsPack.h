#pragma once
#include "LibIncludes.h"

namespace sds
{
	/**
	 * \brief A data structure to hold player information. A default constructed KeyboardPlayerInfo struct has default values that are usable. Thread safe.
	 * \remarks Thread-safe.
	 */
	struct KeyboardPlayerInfo
	{
		std::atomic<int> player_id{ 0 };
		KeyboardPlayerInfo() = default;
		//copy constructor
		KeyboardPlayerInfo(const KeyboardPlayerInfo& sp)
		{
			player_id.exchange(sp.player_id);
		}
		//assignment
		KeyboardPlayerInfo& operator=(const KeyboardPlayerInfo& sp)
		{
			if (this == &sp)
				return *this;
			player_id.exchange(sp.player_id);
			return *this;
		}
		//move constructor
		KeyboardPlayerInfo(KeyboardPlayerInfo&& other) noexcept = delete;
		//Move assignment operator
		auto operator=(KeyboardPlayerInfo other)->KeyboardPlayerInfo & = delete;
		~KeyboardPlayerInfo() = default;
	};

	/**
	 * \brief Some constants that might someday be configurable.
	 */
	struct KeyboardSettings
	{
		//Input Poller maximum number of XINPUT_KEYSTROKE structs to queue before dropping input.
		static constexpr int MAX_STATE_COUNT{ 1'000 };
		//Input Poller thread delay, in milliseconds.
		static constexpr int THREAD_DELAY_POLLER{ 10 };
		//Microseconds Delay Keyrepeat is the time delay a button has in between activations.
		static constexpr std::chrono::microseconds MICROSECONDS_DELAY_KEYREPEAT{ 100'000 };
		//It is necessary to be able to distinguish these mapping values in KeyboardTranslator.
		static constexpr std::array<int, 8> THUMBSTICK_L_VK_LIST
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
		static constexpr std::array<int, 8> THUMBSTICK_R_VK_LIST
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

	/**
	 * \brief For no other reason but to make the common task of injecting these down the architecture less verbose.
	 */
	struct KeyboardSettingsPack
	{
		KeyboardPlayerInfo playerInfo;
		KeyboardSettings settings;
	};
}
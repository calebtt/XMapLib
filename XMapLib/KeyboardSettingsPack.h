#pragma once
#include "stdafx.h"

namespace sds
{
	/// <summary> A data structure to hold player information. A default constructed KeyboardPlayerInfo
	/// struct has default values that are usable. Thread safe. </summary>
	///	<remarks>Thread safe.</remarks>
	struct KeyboardPlayerInfo
	{
		//ISO CPP guidelines C.45 followed here: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-default
		std::atomic<int> player_id{ 0 };
		//default ctor
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
		KeyboardPlayerInfo(KeyboardPlayerInfo&& sp) = delete;
		//Move assignment operator
		KeyboardPlayerInfo& operator=(KeyboardPlayerInfo&& sp) = delete;
		~KeyboardPlayerInfo() = default;
	};
	/// <summary> Some constants that might someday be configurable. </summary>
	struct KeyboardSettings
	{
		//Input Poller maximum number of XINPUT_KEYSTROKE structs to queue before dropping input.
		static constexpr int MAX_STATE_COUNT{ 1'000 };
		//Input Poller thread delay, in milliseconds.
		static constexpr int THREAD_DELAY_POLLER{ 10 };
		//Microseconds Delay Keyrepeat is the time delay a button has in between activations.
		static constexpr int MICROSECONDS_DELAY_KEYREPEAT{ 100'000 };
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
	///<summary>For no other reason but to make the common task of injecting these down the architecture less verbose. </summary>
	struct KeyboardSettingsPack
	{
		KeyboardPlayerInfo playerInfo;
		KeyboardSettings settings;
	};
}
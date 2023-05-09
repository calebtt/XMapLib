#pragma once
#include "LibIncludes.h"

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
	 * \brief Some constants that might someday be configurable.
	 */
	struct KeyboardSettings
	{
		//Input Poller thread delay, in milliseconds.
		static constexpr int THREAD_DELAY_POLLER{ 10 };
		//Microseconds Delay Keyrepeat is the time delay a button has in between activations.
		static constexpr std::chrono::microseconds MICROSECONDS_DELAY_KEYREPEAT{ 100'000 };
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
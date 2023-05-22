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
		// Delay each iteration of a polling loop, short enough to not miss information,
		// long enough to not waste CPU cycles.
		static constexpr detail::NanosDelay_t PollingLoopDelay{ std::chrono::milliseconds{1} };
		//Key Repeat Delay is the time delay a button has in-between activations.
		static constexpr detail::NanosDelay_t KeyRepeatDelay{ std::chrono::microseconds{100'000} };
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
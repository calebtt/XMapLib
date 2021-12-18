#pragma once
#include "stdafx.h"
#include <mutex>

namespace sds
{
	/// <summary>
	/// Some constants that might someday be configurable.
	/// </summary>
	struct KeyboardSettings
	{
		//Input Poller thread delay, in milliseconds.
		constexpr static const int THREAD_DELAY_POLLER = 10;
		//Microseconds Delay Keyrepeat is the time delay a button has in between activations.
		constexpr static const int MICROSECONDS_DELAY_KEYREPEAT = 100000;
	};
}


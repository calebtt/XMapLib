#pragma once
#include "LibIncludes.h"
// TODO, perhaps not in this iteration, but eventually the "mapping" will just be a callback function.
// It would just be more flexible for whatever one might want to do with the code. Lambdas are cheap these days!
namespace sds
{
	/**
	 * \brief Wrapper for XINPUT_KEYSTROKE in this case, to make the code processing it more portable.
	 */
	struct ControllerStateWrapper
	{
		//virtual keycode for controller button activity.
		unsigned short VirtualKey{};
		//unsigned short Flags{};
		bool KeyDown{ false };
		bool KeyUp{ false };
		bool KeyRepeat{ false };
	};
}
#pragma once
#include "LibIncludes.h"

namespace sds
{
	/**
	 * \brief Wrapper for XINPUT_KEYSTROKE in this case, to make the code processing it more portable.
	 */
	struct ControllerStateWrapper
	{
		//virtual keycode for controller button activity.
		unsigned short VirtualKey{};
		bool KeyDown{ false };
		bool KeyUp{ false };
		bool KeyRepeat{ false };
	};
}
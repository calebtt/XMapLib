#pragma once
#include "LibIncludes.h"

namespace sds
{
	/// <summary> Wrapper for XINPUT_STATE thumbstick values in this case,
	/// to make the code processing it more portable. </summary>
	struct ThumbStateWrapper
	{
		using ShortType = short;
		ShortType RightThumbX{};
		ShortType RightThumbY{};
		ShortType LeftThumbX{};
		ShortType LeftThumbY{};
	};
}

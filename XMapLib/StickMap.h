#pragma once
#include "stdafx.h"
namespace sds
{
	/// <summary>
	/// Used to denote which thumbstick is to be used for mouse movement.
	///	This is not really a utility in the general sense, but for Xinmapper it is.
	/// </summary>
	enum class StickMap : int
	{
		NEITHER_STICK,
		RIGHT_STICK,
		LEFT_STICK
	};
}

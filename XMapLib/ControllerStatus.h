#pragma once
#include "stdafx.h"

namespace sds
{
	struct ControllerStatus
	{
		[[nodiscard]] static bool IsControllerConnected(int pid) noexcept
		{
			XINPUT_KEYSTROKE ss{};
			const DWORD ret = XInputGetKeystroke(pid, 0, &ss);
			return ret == ERROR_SUCCESS || ret == ERROR_EMPTY;
		}
	};
}
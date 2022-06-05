#pragma once
#include "stdafx.h"

namespace sds
{
	/// <summary> Object with static method for returning the controller connected status. </summary>
	struct ControllerStatus
	{
		[[nodiscard]] static bool IsControllerConnected(const int pid) noexcept
		{
			XINPUT_KEYSTROKE ss{};
			XINPUT_STATE xs{};
			const auto xsRet = XInputGetState(pid, &xs);
			const DWORD ret = XInputGetKeystroke(pid, 0, &ss);
			return (ret == ERROR_SUCCESS || ret == ERROR_EMPTY) && (xsRet == ERROR_SUCCESS || xsRet == ERROR_EMPTY);
		}
	};
}
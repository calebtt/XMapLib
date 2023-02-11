#pragma once
#include "pch.h"

namespace sds
{
	/// <summary> Object with static method for returning the controller connected status. </summary>
	struct ControllerStatus
	{
		[[nodiscard]] static bool IsControllerConnected(const int pid) noexcept
		{
			XINPUT_KEYSTROKE keystrokeObj{};
			XINPUT_STATE stateObj{};
			const auto xsRet = XInputGetState(pid, &stateObj);
			const DWORD ret = XInputGetKeystroke(pid, 0, &keystrokeObj);
			return (ret == ERROR_SUCCESS || ret == ERROR_EMPTY) && (xsRet == ERROR_SUCCESS || xsRet == ERROR_EMPTY);
		}
	};
}
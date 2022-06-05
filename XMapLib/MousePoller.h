#pragma once
#include "stdafx.h"
#include "ThumbStateWrapper.h"

namespace sds
{
	/// <summary> Encapsulates the logic for querying the OS to gather information about a controller thumbstick event. </summary>
	class MousePoller
	{
		using LogFnType = std::function<void(const char*)>;
		const LogFnType m_logFn;
		XINPUT_STATE m_tempState{};
	public:
		MousePoller(LogFnType logFn = nullptr) : m_logFn(logFn)
		{ }
		/// <summary>Returns an updated ThumbStateWrapper containing information gathered about a controller keypress. </summary>
		ThumbStateWrapper GetUpdatedState(const int playerId) noexcept
		{
			// zero controller state struct
			m_tempState = {};
			// get updated controller state information
			const DWORD error = XInputGetState(playerId, &m_tempState);
			if (error == ERROR_SUCCESS || error == ERROR_EMPTY)
			{
				return ThumbStateWrapper {
					.RightThumbX = m_tempState.Gamepad.sThumbRX,
					.RightThumbY = m_tempState.Gamepad.sThumbRY,
					.LeftThumbX = m_tempState.Gamepad.sThumbLX,
					.LeftThumbY = m_tempState.Gamepad.sThumbLY
				};
			}
			if (error == ERROR_BAD_ARGUMENTS)
			{
				if (m_logFn != nullptr)
					m_logFn("Exception in MousePoller::GetUpdatedState(...): Call to XInputGetState() reported ERROR_BAD_ARGUMENTS");
			}
			return {};
		}
	};
}
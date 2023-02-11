#pragma once
#include "LibIncludes.h"
#include "ThumbStateWrapper.h"

namespace sds
{
	/// <summary> Encapsulates the logic for querying the OS to gather information about a controller thumbstick event. </summary>
	class MousePoller
	{
		XINPUT_STATE m_tempState{};
	public:
		/// <summary>Returns an updated ThumbStateWrapper containing information gathered about a controller keypress. </summary>
		[[nodiscard]]
		auto GetUpdatedState(const int playerId) noexcept -> std::optional<ThumbStateWrapper>
		{
			// zero controller state struct
			m_tempState = {};
			// get updated controller state information
			const auto error = XInputGetState(playerId, &m_tempState);
			if (error == ERROR_SUCCESS || error == ERROR_EMPTY)
			{
				return ThumbStateWrapper {
					.RightThumbX = m_tempState.Gamepad.sThumbRX,
					.RightThumbY = m_tempState.Gamepad.sThumbRY,
					.LeftThumbX = m_tempState.Gamepad.sThumbLX,
					.LeftThumbY = m_tempState.Gamepad.sThumbLY
				};
			}
			return {};
		}
	};
}
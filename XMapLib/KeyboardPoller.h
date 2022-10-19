#pragma once
#include "stdafx.h"
#include "KeyStateWrapper.h"

namespace sds
{
	/// <summary> Encapsulates the logic for querying the OS to gather information about a controller keypress event. </summary>
	class KeyboardPoller
	{
		XINPUT_KEYSTROKE m_tempState{};
	public:
		/// <summary>Returns an updated KeyStateWrapper containing information gathered about a controller keypress. </summary>
		KeyStateWrapper GetUpdatedState(const int playerId) noexcept
		{
			// zero controller state struct
			m_tempState = {};
			// get updated controller state information
			const DWORD error = XInputGetKeystroke(playerId, 0, &m_tempState);
			if (error == ERROR_SUCCESS || error == ERROR_EMPTY)
			{
				return KeyStateWrapper{ .VirtualKey = m_tempState.VirtualKey, .Flags = m_tempState.Flags };
			}
			assert(error != ERROR_BAD_ARGUMENTS);
			return {};
		}
	};
}
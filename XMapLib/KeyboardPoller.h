#pragma once
#include "stdafx.h"

namespace sds
{
	/// <summary> Encapsulates the logic for querying the OS to gather information about a controller keypress event. </summary>
	class KeyboardPoller
	{
		using LogFnType = std::function<void(std::string)>;
		const LogFnType m_logFn;
		XINPUT_KEYSTROKE m_tempState{};
	public:
		KeyboardPoller(LogFnType logFn = nullptr) : m_logFn(logFn)
		{ }
		/// <summary> Wrapper for XINPUT_KEYSTROKE in this case, to make the code processing it more portable. </summary>
		struct KeyStateWrapper
		{
			//virtual keycode for controller button activity.
			unsigned short VirtualKey{};
			unsigned short Flags{};
		};
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
			if(error == ERROR_BAD_ARGUMENTS)
			{
				if (m_logFn != nullptr)
					m_logFn("Exception in KeyboardPoller::GetUpdatedState(...): Call to XInputGetKeystroke() reported ERROR_BAD_ARGUMENTS");
			}
			return {};
		}
	};
}
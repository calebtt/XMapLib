#pragma once
#include "LibIncludes.h"
#include "ControllerStateWrapper.h"

namespace sds
{
	/**
	 * \brief Encapsulates the logic for querying the OS to gather information about a controller keypress event.
	 * \remarks NOTE if the controller state info API you are using does not have a "repeat" state event, then you will
	 * need to track down/up status yourself in this class in order to send it for processing--to have key repeat behavior.
	 */
	class KeyboardPollerController
	{
		XINPUT_KEYSTROKE m_tempState{};
		int m_playerId{};
	public:
		KeyboardPollerController() = default;
		KeyboardPollerController(const int pid) : m_playerId(pid) { }
	public:
		/**
		 * \brief Returns an updated ControllerStateWrapper containing information gathered about a controller keypress.
		 */
		[[nodiscard]]
		auto operator()() noexcept -> ControllerStateWrapper
		{
			return GetUpdatedState();
		}

		/**
		 * \brief Returns an updated ControllerStateWrapper containing information gathered about a controller keypress.
		 */
		[[nodiscard]]
		auto GetUpdatedState() noexcept -> ControllerStateWrapper
		{
			// zero controller state struct
			m_tempState = {};
			const DWORD error = XInputGetKeystroke(m_playerId, 0, &m_tempState);
			if (error == ERROR_SUCCESS || error == ERROR_EMPTY)
			{
				const bool isDown = m_tempState.Flags & XINPUT_KEYSTROKE_KEYDOWN;
				const bool isUp = m_tempState.Flags & XINPUT_KEYSTROKE_KEYUP;
				const bool isRepeat = m_tempState.Flags & XINPUT_KEYSTROKE_REPEAT;
				return ControllerStateWrapper{ .VirtualKey = m_tempState.VirtualKey, .KeyDown = isDown, .KeyUp = isUp, .KeyRepeat = isRepeat };
			}
			assert(error != ERROR_BAD_ARGUMENTS);
			return {};
		}
	};
}
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
	class KeyboardPoller
	{
		static constexpr std::size_t StateQueueSize{ 16 };
		using StateQueue_t = std::array<ControllerStateWrapper, StateQueueSize>;
		XINPUT_KEYSTROKE m_tempState{};
		StateQueue_t m_stateQueue{};
		int m_playerId{};
	public:
		KeyboardPoller() = default;
		KeyboardPoller(const int pid) : m_playerId(pid) { }
	public:
		auto operator()() -> ControllerStateWrapper
		{
			return GetUpdatedState();
		}

		/**
		 * \brief Returns an updated ControllerStateWrapper containing information gathered about a controller keypress.
		 * \param playerId controller/player ID value
		 */
		[[nodiscard]]
		auto GetUpdatedState(const int playerId) noexcept -> ControllerStateWrapper
		{
			// zero controller state struct
			m_tempState = {};
			// get updated controller state information
			const DWORD error = XInputGetKeystroke(playerId, 0, &m_tempState);
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

		/**
		 * \brief Returns an updated ControllerStateWrapper containing information gathered about a controller keypress using
		 * the player id set at construction.
		 */
		[[nodiscard]]
		auto GetUpdatedState() noexcept -> ControllerStateWrapper
		{
			return GetUpdatedState(m_playerId);
		}

		/**
		 * \brief Returns an updated ControllerStateWrapper buffer containing information gathered about controller keypresses.
		 * \param playerId controller/player ID value
		 */
		[[nodiscard]]
		auto GetUpdatedStateQueue(const int playerId) noexcept -> std::array<ControllerStateWrapper, StateQueueSize>
		{
			m_stateQueue = {};
			// get updated controller state information
			for(auto& it : m_stateQueue)
			{
				m_tempState = {};
				const DWORD error = XInputGetKeystroke(playerId, 0, &m_tempState);
				if (error == ERROR_SUCCESS || error == ERROR_EMPTY)
				{
					const bool isDown = m_tempState.Flags & XINPUT_KEYSTROKE_KEYDOWN;
					const bool isUp = m_tempState.Flags & XINPUT_KEYSTROKE_KEYUP;
					const bool isRepeat = m_tempState.Flags & XINPUT_KEYSTROKE_REPEAT;
					it = ControllerStateWrapper{ .VirtualKey = m_tempState.VirtualKey, .KeyDown = isDown, .KeyUp = isUp, .KeyRepeat = isRepeat };
				}
				assert(error != ERROR_BAD_ARGUMENTS);
			}
			return m_stateQueue;
		}

		/**
		 * \brief Returns an updated ControllerStateWrapper buffer containing information gathered about controller keypresses using
		 * the player id set at construction.
		 */
		[[nodiscard]]
		auto GetUpdatedStateQueue() noexcept -> std::array<ControllerStateWrapper, StateQueueSize>
		{
			return GetUpdatedStateQueue(m_playerId);
		}
	};
}
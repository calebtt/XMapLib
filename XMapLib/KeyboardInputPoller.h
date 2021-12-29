#pragma once
#include "stdafx.h"
#include "Utilities.h"

namespace sds
{
	/// <summary>
	/// Polls for input from the XInput library in it's worker thread function.
	/// Values are used in KeyboardMapper, the main class for use.
	/// </summary>
	class KeyboardInputPoller : public CPPThreadRunner<std::vector<XINPUT_KEYSTROKE>>
	{
		KeyboardPlayerInfo m_local_player;
		const int EMPTY_COUNT = 5000;
	protected:
		/// <summary>
		/// Worker thread overriding the base pure virtual workThread.
		///	Updates "m_local_state" with mutex protection.
		/// </summary>
		void workThread() override
		{
			this->m_is_thread_running = true;
			XINPUT_KEYSTROKE tempState = {};
			int currentCount = 0;
			while (!this->m_is_stop_requested)
			{
				memset(&tempState, 0, sizeof(tempState));
				const DWORD error = XInputGetKeystroke(m_local_player.player_id, 0, &tempState);
				if (error == ERROR_SUCCESS)
				{
					addElement(tempState);
					//Utilities::LogError(std::to_string(tempState.VirtualKey));
				}
				else if(error == ERROR_EMPTY)
				{
					currentCount++;
					if(currentCount > EMPTY_COUNT)
					{
						currentCount = 0;
						addElement(tempState);
					}
				}
				//std::this_thread::sleep_for(std::chrono::milliseconds(KeyboardSettings::THREAD_DELAY_POLLER));
			}
			this->m_is_thread_running = false;
		}
		void updateState(const std::vector<XINPUT_KEYSTROKE>& state) = delete; //explicitly hidden base member
		std::vector<XINPUT_KEYSTROKE> getCurrentState() = delete; //explicitly hidden base member
		void addElement(const XINPUT_KEYSTROKE &state)
		{
			lock addLock(m_state_mutex);
			if (m_local_state.size() < KeyboardSettings::MAX_STATE_COUNT)
				m_local_state.push_back(state);
			else
				Utilities::LogError("KeyboardInputPoller::addElement(): State buffer dropping states.");
		}
		std::vector<XINPUT_KEYSTROKE> getAndClearStates()
		{
			lock retrieveLock(m_state_mutex);
			std::vector<XINPUT_KEYSTROKE> ret(m_local_state);
			m_local_state.clear();
			return ret;
		}
	public:
		KeyboardInputPoller()
		{
			Start();
		}
		explicit KeyboardInputPoller(KeyboardPlayerInfo& p) : m_local_player(p) { }
		KeyboardInputPoller(const KeyboardInputPoller& other) = delete;
		KeyboardInputPoller(KeyboardInputPoller&& other) = delete;
		KeyboardInputPoller& operator=(const KeyboardInputPoller& other) = delete;
		KeyboardInputPoller& operator=(KeyboardInputPoller&& other) = delete;
		/// <summary>
		/// Destructor override, ensures the running thread function is stopped
		/// inside of this class and not the base.
		/// </summary>
		~KeyboardInputPoller() override
		{
			Stop();
		}
		auto GetUpdatedState()
		{
			return getAndClearStates();
		}
		/// <summary>
		/// Start polling for updated XINPUT_STATE info.
		/// </summary>
		void Start()
		{
			this->startThread();
		}
		/// <summary>
		/// Stop input polling.
		/// </summary>
		void Stop()
		{
			this->stopThread();
		}
		/// <summary>
		/// Gets the running status of the worker thread
		/// </summary>
		/// <returns> true if thread is running, false otherwise</returns>
		bool IsRunning() const
		{
			return this->m_is_thread_running;
		}
		/// <summary>
		/// Returns status of XINPUT library detecting a controller.
		/// </summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		bool IsControllerConnected() const
		{
			XINPUT_KEYSTROKE ss = {};
			memset(&ss, 0, sizeof(ss));
			const DWORD ret = XInputGetKeystroke(m_local_player.player_id,0, &ss);
			return ret == ERROR_SUCCESS || ret == ERROR_EMPTY;
		}
		/// <summary>
		/// Returns status of XINPUT library detecting a controller.
		/// overload that uses the player_id value in a KeyboardPlayerInfo struct
		/// </summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		bool IsControllerConnected(const KeyboardPlayerInfo& p) const
		{
			XINPUT_KEYSTROKE ss = {};
			memset(&ss, 0, sizeof(ss));
			const DWORD ret = XInputGetKeystroke(p.player_id, 0, &ss);
			return ret == ERROR_SUCCESS || ret == ERROR_EMPTY;
		}
	};

}
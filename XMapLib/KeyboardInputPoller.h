#pragma once
#include "stdafx.h"
#include "Utilities.h"

namespace sds
{
	/// <summary>
	/// Polls for input from the XInput library in it's worker thread function,
	/// sends them to MouseMapper and KeyboardTranslator for processing.
	/// </summary>
	class KeyboardInputPoller : public CPPThreadRunner<XINPUT_KEYSTROKE>
	{
		KeyboardPlayerInfo m_local_player;
	protected:
		/// <summary>
		/// Worker thread overriding the base pure virtual workThread.
		///	Updates "m_local_state" with mutex protection.
		/// </summary>
		void workThread() override
		{
			this->m_is_thread_running = true;
			{
				//zero m_local_state before use
				lock first(m_state_mutex);
				memset(&m_local_state, 0, sizeof(m_local_state));
			}
			XINPUT_KEYSTROKE tempState = {};
			while (!this->m_is_stop_requested)
			{
				memset(&tempState, 0, sizeof(tempState));
				const DWORD error = XInputGetKeystroke(m_local_player.player_id, 0, &tempState);
				if (error == ERROR_SUCCESS)
				{
					updateState(tempState);
					//Utilities::XErrorLogger::LogError(std::to_string(tempState.VirtualKey));
				}
				//std::this_thread::sleep_for(std::chrono::milliseconds(KeyboardSettings::THREAD_DELAY_POLLER));
			}
			this->m_is_thread_running = false;
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
		XINPUT_KEYSTROKE GetUpdatedState()
		{
			return getCurrentState();
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
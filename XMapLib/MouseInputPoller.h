#pragma once
#include "stdafx.h"
#include "Utilities.h"

namespace sds
{
	/// <summary>
	/// Polls for input from the XInput library in it's worker thread function.
	/// Values are used in MouseMapper, the main class for use.
	/// </summary>
	class MouseInputPoller : public CPPThreadRunner<XINPUT_STATE>
	{
		MousePlayerInfo m_local_player;
	protected:
		/// <summary>
		/// Worker thread overriding the base pure virtual workThread.
		///	Updates "local_state" with mutex protection.
		/// </summary>
		void workThread() override
		{
			this->m_is_thread_running = true;
			{
				//zero local_state before use
				lock first(m_state_mutex);
				memset(&m_local_state, 0, sizeof(XINPUT_STATE));
			}
			XINPUT_STATE tempState = {};
			DWORD lastPacket = 0;
			while( !this->m_is_stop_requested)
			{
				memset(&tempState, 0, sizeof(XINPUT_STATE));
				const DWORD error = XInputGetState(m_local_player.player_id, &tempState);
				if (error == ERROR_SUCCESS)
				{
					if(tempState.dwPacketNumber != lastPacket)
					{
						lastPacket = tempState.dwPacketNumber;
						lock second(m_state_mutex);
						m_local_state = tempState;
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(MouseSettings::THREAD_DELAY_POLLER));
			}
			this->m_is_thread_running = false;
		}
	public:
		MouseInputPoller()
		{
			Start();
		}
		explicit MouseInputPoller(MousePlayerInfo &p) : m_local_player(p) { }
		MouseInputPoller(const MouseInputPoller& other) = delete;
		MouseInputPoller(MouseInputPoller&& other) = delete;
		MouseInputPoller& operator=(const MouseInputPoller& other) = delete;
		MouseInputPoller& operator=(MouseInputPoller&& other) = delete;
		/// <summary>
		/// Destructor override, ensures the running thread function is stopped
		/// inside of this class and not the base.
		/// </summary>
		~MouseInputPoller() override
		{
			Stop();
		}
		XINPUT_STATE GetUpdatedState()
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
			XINPUT_STATE ss = {};
			memset(&ss, 0, sizeof(XINPUT_STATE));
			return XInputGetState(m_local_player.player_id, &ss) == ERROR_SUCCESS;
		}
		/// <summary>
		/// Returns status of XINPUT library detecting a controller.
		/// overload that uses the player_id value in a MousePlayerInfo struct
		/// </summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		bool IsControllerConnected(const MousePlayerInfo &p) const
		{
			XINPUT_STATE ss = {};
			memset(&ss, 0, sizeof(ss));
			return XInputGetState(p.player_id, &ss) == ERROR_SUCCESS;
		}
	};

}
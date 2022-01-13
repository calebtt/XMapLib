#pragma once
#include "stdafx.h"
#include "Utilities.h"

namespace sds
{
	/// <summary>
	/// Polls for input from the XInput library in it's worker thread function.
	/// Values are used in MouseMapper, the main class for use.
	/// </summary>
	class MouseInputPoller
	{
		MousePlayerInfo m_local_player;
		std::unique_ptr<sds::CPPLambdaRunner<XINPUT_STATE>> m_workThread;
		void InitWorkThread() noexcept
		{
			m_workThread =
				std::make_unique<sds::CPPLambdaRunner<XINPUT_STATE>>
				([this](sds::LambdaArgs::LambdaArg1& stopCondition, sds::LambdaArgs::LambdaArg2& mut, XINPUT_STATE& protectedData) { workThread(stopCondition, mut, protectedData); });
		}
	public:
		MouseInputPoller()
		{
			InitWorkThread();
			Start();
		}
		explicit MouseInputPoller(MousePlayerInfo &p) : m_local_player(p)
		{
			InitWorkThread();
			Start();
		}
		MouseInputPoller(const MouseInputPoller& other) = delete;
		MouseInputPoller(MouseInputPoller&& other) = delete;
		MouseInputPoller& operator=(const MouseInputPoller& other) = delete;
		MouseInputPoller& operator=(MouseInputPoller&& other) = delete;
		/// <summary>
		/// Destructor override, ensures the running thread function is stopped
		/// inside of this class and not the base.
		/// </summary>
		~MouseInputPoller()
		{
			Stop();
		}
		XINPUT_STATE GetUpdatedState() noexcept
		{
			if (m_workThread)
				return m_workThread->GetCurrentState();
			return XINPUT_STATE{};
		}
		/// <summary>
		/// Start polling for updated XINPUT_STATE info.
		/// </summary>
		void Start()
		{
			if (m_workThread)
				m_workThread->StartThread();
		}
		/// <summary>
		/// Stop input polling.
		/// </summary>
		void Stop()
		{
			if (m_workThread)
				m_workThread->StopThread();
		}
		/// <summary>
		/// Gets the running status of the worker thread
		/// </summary>
		/// <returns> true if thread is running, false otherwise</returns>
		bool IsRunning() const
		{
			if(m_workThread)
				return m_workThread->IsRunning();
			return false;
		}
		/// <summary>
		/// Returns status of XINPUT library detecting a controller.
		/// </summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		bool IsControllerConnected() const
		{
			XINPUT_STATE ss{};
			return XInputGetState(m_local_player.player_id, &ss) == ERROR_SUCCESS;
		}
		/// <summary>
		/// Returns status of XINPUT library detecting a controller.
		/// overload that uses the player_id value in a MousePlayerInfo struct
		/// </summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		bool IsControllerConnected(const MousePlayerInfo &p) const
		{
			XINPUT_STATE ss{};
			return XInputGetState(p.player_id, &ss) == ERROR_SUCCESS;
		}
	protected:
		/// <summary>
		/// Worker thread overriding the base pure virtual workThread.
		///	Updates "local_state" with mutex protection.
		/// </summary>
		void workThread(sds::LambdaArgs::LambdaArg1& stopCondition, sds::LambdaArgs::LambdaArg2& mut, XINPUT_STATE& protectedData)
		{
			using lock = CPPLambdaRunner<XINPUT_STATE>::ScopedLockType;
			{
				//zero local_state before use
				lock first(mut);
				protectedData = { 0 };
			}
			XINPUT_STATE tempState = {};
			DWORD lastPacket = 0;
			while (!stopCondition)
			{
				tempState = { 0 };
				const DWORD error = XInputGetState(m_local_player.player_id, &tempState);
				if (error == ERROR_SUCCESS)
				{
					if (tempState.dwPacketNumber != lastPacket)
					{
						lastPacket = tempState.dwPacketNumber;
						lock second(mut);
						protectedData = tempState;
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(MouseSettings::THREAD_DELAY_POLLER));
			}
		}
	};

}
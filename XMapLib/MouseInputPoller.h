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
		using InternalType = XINPUT_STATE;
		using LambdaRunnerType = sds::CPPRunnerGeneric<InternalType>;
		using lock = LambdaRunnerType::ScopedLockType;
		inline static constexpr size_t MaxUnhandledBeforeSleep{ 250 };
		MousePlayerInfo m_local_player{};
		LambdaRunnerType m_workThread;
		auto GetLambda()
		{
			return [this](const auto stopCondition, const auto mut, auto protectedData) { workThread(stopCondition, mut, protectedData); };
		}
	public:
		MouseInputPoller() : m_workThread(GetLambda())
		{

		}
		explicit MouseInputPoller(const MousePlayerInfo &p) : m_local_player(p), m_workThread(GetLambda())
		{

		}
		MouseInputPoller(const MouseInputPoller& other) = delete;
		MouseInputPoller(MouseInputPoller&& other) = delete;
		MouseInputPoller& operator=(const MouseInputPoller& other) = delete;
		MouseInputPoller& operator=(MouseInputPoller&& other) = delete;
		~MouseInputPoller() = default;

		[[nodiscard]] XINPUT_STATE GetUpdatedState() noexcept
		{
			return m_workThread.GetCurrentState();
		}
		/// <summary>Start polling for updated XINPUT_STATE info.</summary>
		void Start() noexcept
		{
			m_workThread.StartThread();
		}
		/// <summary>Stop input polling. Blocks and waits for finish.</summary>
		void Stop() noexcept
		{
			m_workThread.StopThread();
		}
		/// <summary>Gets the running status of the worker thread</summary>
		/// <returns> true if thread is running, false otherwise</returns>
		[[nodiscard]] bool IsRunning() const noexcept
		{
			return m_workThread.IsRunning();
		}
		/// <summary>Returns status of XINPUT library detecting a controller.</summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		[[nodiscard]] bool IsControllerConnected() const noexcept
		{
			XINPUT_STATE ss{};
			return XInputGetState(m_local_player.player_id, &ss) == ERROR_SUCCESS;
		}
		/// <summary>Returns status of XINPUT library detecting a controller.
		/// This overload uses the player_id value in a MousePlayerInfo struct.</summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		[[nodiscard]] bool IsControllerConnected(const MousePlayerInfo &p) const noexcept
		{
			XINPUT_STATE ss{};
			return XInputGetState(p.player_id, &ss) == ERROR_SUCCESS;
		}
	protected:
		/// <summary>Worker thread used by m_workThread. Updates the protectedData with mutex protection.</summary>
		void workThread(const auto stopCondition, const auto mut, const auto protectedData) const noexcept
		{
			Utilities::TPrior tp;
			if (!tp.SetPriorityLow())
				Utilities::LogError("Failed to set thread priority in MouseInputPoller::workThread(auto,auto,auto)");
			//local scope here
			{
				//zero local_state before use
				lock first(*mut);
				*protectedData = {};
			}
			XINPUT_STATE tempState{};
			DWORD lastPacket = 0;
			size_t unhandledCount{ 0 };
			while (!(*stopCondition))
			{
				tempState = {};
				const DWORD error = XInputGetState(m_local_player.player_id, &tempState);
				if (error == ERROR_SUCCESS)
				{
					if (tempState.dwPacketNumber != lastPacket)
					{
						lastPacket = tempState.dwPacketNumber;
						lock second(*mut);
						*protectedData = tempState;
					}
					unhandledCount = 0;
				}
				else
				{
					if(unhandledCount < MaxUnhandledBeforeSleep)
						unhandledCount++;
				}
				if (unhandledCount > MaxUnhandledBeforeSleep)
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
				std::this_thread::sleep_for(std::chrono::milliseconds(MouseSettings::THREAD_DELAY_INPUT_POLLER_MS));
			}
		}
	};

}
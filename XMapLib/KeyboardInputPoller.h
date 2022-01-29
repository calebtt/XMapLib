#pragma once
#include "stdafx.h"
#include "Utilities.h"

namespace sds
{
	/// <summary>
	/// Polls for input from the XInput library in it's worker thread function.
	/// Values are used in KeyboardMapper, the main class for use.
	/// </summary>
	class KeyboardInputPoller
	{
		using InternalType = std::vector<XINPUT_KEYSTROKE>;
		using LambdaRunnerType = sds::CPPRunnerGeneric<InternalType>;
		using lock = LambdaRunnerType::ScopedLockType;
		const int EMPTY_COUNT = 5000;
		KeyboardPlayerInfo m_local_player;
		std::unique_ptr<LambdaRunnerType> m_workThread;
		void InitWorkThread() noexcept
		{
			m_workThread =
				std::make_unique<LambdaRunnerType>
				([this](auto& stopCondition, auto& mut, auto& protectedData) { workThread(stopCondition, mut, protectedData); });
		}
	public:
		KeyboardInputPoller()
		{
			InitWorkThread();
			Start();
		}
		explicit KeyboardInputPoller(const KeyboardPlayerInfo& p) : m_local_player(p) { InitWorkThread(); Start(); }
		KeyboardInputPoller(const KeyboardInputPoller& other) = delete;
		KeyboardInputPoller(KeyboardInputPoller&& other) = delete;
		KeyboardInputPoller& operator=(const KeyboardInputPoller& other) = delete;
		KeyboardInputPoller& operator=(KeyboardInputPoller&& other) = delete;
		~KeyboardInputPoller() = default;

		/// <summary>Returns copy and clears internal one.</summary>
		std::vector<XINPUT_KEYSTROKE> getAndClearStates() const
		{
			return m_workThread->GetAndClearCurrentStates();
		}
		/// <summary>Start polling for updated XINPUT_KEYSTROKE info.</summary>
		void Start() const noexcept
		{
			if (m_workThread)
				m_workThread->StartThread();
		}
		/// <summary>Stop input polling.</summary>
		void Stop() const noexcept
		{
			if (m_workThread)
				m_workThread->StopThread();
		}
		/// <summary>Gets the running status of the worker thread</summary>
		/// <returns> true if thread is running, false otherwise</returns>
		bool IsRunning() const noexcept
		{
			if (m_workThread)
				return m_workThread->IsRunning();
			return false;
		}
		/// <summary>Returns status of XINPUT library detecting a controller.</summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		bool IsControllerConnected() const noexcept
		{
			XINPUT_KEYSTROKE ss{};
			const DWORD ret = XInputGetKeystroke(m_local_player.player_id,0, &ss);
			return ret == ERROR_SUCCESS || ret == ERROR_EMPTY;
		}
		/// <summary>Returns status of XINPUT library detecting a controller.
		/// This overload uses the player_id value in a KeyboardPlayerInfo struct</summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		bool IsControllerConnected(const KeyboardPlayerInfo& p) const noexcept
		{
			XINPUT_KEYSTROKE ss{};
			const DWORD ret = XInputGetKeystroke(p.player_id, 0, &ss);
			return ret == ERROR_SUCCESS || ret == ERROR_EMPTY;
		}
	protected:
		/// <summary>Worker thread used by m_workThread. Updates the protectedData with mutex protection.</summary>
		void workThread(sds::LambdaArgs::LambdaArg1& stopCondition, sds::LambdaArgs::LambdaArg2& mut, auto& protectedData)
		{
			auto addElement = [this,&mut,&protectedData](const XINPUT_KEYSTROKE& state)
			{
				lock addLock(mut);
				if (protectedData.size() < KeyboardSettings::MAX_STATE_COUNT)
					protectedData.push_back(state);
				else
					Utilities::LogError("KeyboardInputPoller::addElement(): State buffer dropping states.");
			};
			XINPUT_KEYSTROKE tempState{};
			int currentCount = 0;
			while (!stopCondition)
			{
				tempState={};
				const DWORD error = XInputGetKeystroke(m_local_player.player_id, 0, &tempState);
				if (error == ERROR_SUCCESS)
				{
					addElement(tempState);
				}
				else if (error == ERROR_EMPTY)
				{
					currentCount++;
					if (currentCount > EMPTY_COUNT)
					{
						currentCount = 0;
						addElement(tempState);
					}
				}
				//std::this_thread::sleep_for(std::chrono::milliseconds(KeyboardSettings::THREAD_DELAY_POLLER));
			}
		}
	};

}
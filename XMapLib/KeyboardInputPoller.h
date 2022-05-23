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
		using DataType = std::vector<XINPUT_KEYSTROKE>;
		using LambdaRunnerType = sds::CPPRunnerGeneric<DataType>;
		using ScopedLockType = LambdaRunnerType::ScopedLockType;
		const int EMPTY_COUNT{ 500 };
		KeyboardPlayerInfo m_local_player{};
		LambdaRunnerType m_workThread;
		auto GetLambda()
		{
			return [this](const auto stopCondition, const auto mut, auto protectedData) { workThread(stopCondition, mut, protectedData); };
		}
		static void SetPriority(void *nativeHandle)
		{
			SetThreadPriority(nativeHandle, THREAD_PRIORITY_TIME_CRITICAL);
		}
	public:
		KeyboardInputPoller() : m_workThread(GetLambda())
		{
			SetPriority(m_workThread.GetNativeHandle());
		}
		explicit KeyboardInputPoller(const KeyboardPlayerInfo& p)
		: m_local_player(p), m_workThread(GetLambda())
		{
			SetPriority(m_workThread.GetNativeHandle());
		}
		KeyboardInputPoller(const KeyboardInputPoller& other) = delete;
		KeyboardInputPoller(KeyboardInputPoller&& other) = delete;
		KeyboardInputPoller& operator=(const KeyboardInputPoller& other) = delete;
		KeyboardInputPoller& operator=(KeyboardInputPoller&& other) = delete;
		~KeyboardInputPoller() = default;

		/// <summary>Returns copy and clears internal one.</summary>
		[[nodiscard]] std::vector<XINPUT_KEYSTROKE> getAndClearStates() const
		{
			return m_workThread.GetAndClearCurrentStates();
		}
		/// <summary>Start polling for updated XINPUT_KEYSTROKE info.</summary>
		void Start() noexcept
		{
			m_workThread.StartThread();
		}
		/// <summary>Stop input polling.</summary>
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
			XINPUT_KEYSTROKE ss{};
			const DWORD ret = XInputGetKeystroke(m_local_player.player_id,0, &ss);
			return ret == ERROR_SUCCESS || ret == ERROR_EMPTY;
		}
		/// <summary>Returns status of XINPUT library detecting a controller.
		/// This overload uses the player_id value in a KeyboardPlayerInfo struct</summary>
		/// <returns> true if controller is connected, false otherwise</returns>
		[[nodiscard]] bool IsControllerConnected(const KeyboardPlayerInfo& p) const noexcept
		{
			XINPUT_KEYSTROKE ss{};
			const DWORD ret = XInputGetKeystroke(p.player_id, 0, &ss);
			return ret == ERROR_SUCCESS || ret == ERROR_EMPTY;
		}
	protected:
		/// <summary>Worker thread used by m_workThread. Updates the protectedData with mutex protection. The copied shared_ptr is intentional.</summary>
		void workThread(const auto stopCondition, const auto mut, auto protectedData)
		{
			auto addElement = [&](const XINPUT_KEYSTROKE& state)
			{
				ScopedLockType addLock(*mut);
				if (protectedData->size() < KeyboardSettings::MAX_STATE_COUNT)
					protectedData->push_back(state);
				else
					Utilities::LogError("KeyboardInputPoller::addElement(): State buffer dropping states.");
			};
			XINPUT_KEYSTROKE tempState{};
			int currentCount = 0;
			while (!(*stopCondition))
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
				//no loop delay is tolerable here, essentially. A missed state is a real problem.
				//but someday with a low cpu usage custom timer, added into this class,
				//it should be possible to get performant microsecond loop delays here.
			}
		}
	};

}
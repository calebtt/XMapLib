#pragma once
#include "stdafx.h"
#include "KeyboardInputPoller.h"
#include "KeyboardTranslator.h"

namespace sds
{
	/// <summary>
	/// Main class for use, for mapping controller input to keyboard input.
	/// Uses KeyboardKeyMap for the details.
	/// </summary>
	class KeyboardMapper
	{
		using InternalType = int;
		using LambdaRunnerType = sds::CPPLambdaVectorRunner<InternalType>;
		using lock = LambdaRunnerType::ScopedLockType;
		sds::KeyboardPlayerInfo m_localPlayerInfo;
		sds::KeyboardInputPoller m_poller;
		sds::KeyboardTranslator m_translator;
		std::unique_ptr<LambdaRunnerType> m_workThread;
		void InitWorkThread() noexcept
		{
			m_workThread =
				std::make_unique<LambdaRunnerType>
				([this](auto& stopCondition, auto& mut, auto& protectedData) { workThread(stopCondition, mut, protectedData); });
		}
	public:
		/// <summary>
		/// Ctor for default configuration
		/// </summary>
		KeyboardMapper()
		{
			InitWorkThread();
			Start();
		}
		/// <summary>
		/// Ctor allows setting a custom KeyboardPlayerInfo
		/// </summary>
		explicit KeyboardMapper(const sds::KeyboardPlayerInfo& player) : m_localPlayerInfo(player)
		{
			InitWorkThread();
			Start();
		}
		KeyboardMapper(const KeyboardMapper& other) = delete;
		KeyboardMapper(KeyboardMapper&& other) = delete;
		KeyboardMapper& operator=(const KeyboardMapper& other) = delete;
		KeyboardMapper& operator=(KeyboardMapper&& other) = delete;
		/// <summary>
		/// Destructor override, ensures the running thread function is stopped
		/// inside of this class and not the base.
		/// </summary>
		~KeyboardMapper()
		{
			Stop();
		}
		bool IsControllerConnected() const
		{
			return m_poller.IsControllerConnected();
		}
		bool IsRunning() const
		{
			return m_poller.IsRunning() && m_workThread->IsRunning();
		}
		void Start() const noexcept
		{
			m_poller.Start();
			m_workThread->StartThread();
		}
		void Stop() const noexcept
		{
			m_poller.Stop();
			m_workThread->StopThread();
		}
		std::string AddMap(KeyboardKeyMap button)
		{
			if (IsRunning())
				Stop();
			std::string er = m_translator.AddKeyMap(button);
			if (er.empty())
				Start();
			return er;
		}
		std::vector<KeyboardKeyMap> GetMaps() const
		{
			return m_translator.GetMaps();
		}
		void ClearMaps()
		{
			m_translator.ClearMap();
		}
	protected:
		/// <summary>
		/// Worker thread, protected visibility.
		/// Accesses the std::atomic m_threadX and m_threadY members.
		/// </summary>
		void workThread(auto& stopCondition, auto&, auto&)
		{
			//thread main loop
			while (!stopCondition)
			{
				const std::vector<XINPUT_KEYSTROKE> states = m_poller.getAndClearStates();
				std::for_each(states.begin(), states.end(), [this](auto& cur)
					{
						m_translator.ProcessKeystroke(cur);
					});
				//m_translator.ProcessKeystroke(m_poller.GetUpdatedState());
				std::this_thread::sleep_for(std::chrono::milliseconds(KeyboardSettings::THREAD_DELAY_POLLER));
			}
		}
	};
}

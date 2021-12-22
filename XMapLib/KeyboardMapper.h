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
	class KeyboardMapper : public CPPThreadRunner<int>
	{
	private:
		sds::KeyboardPlayerInfo m_localPlayerInfo;
		sds::KeyboardInputPoller m_poller;
		sds::KeyboardTranslator m_mapper;
	public:
		/// <summary>
		/// Ctor for default configuration
		/// </summary>
		KeyboardMapper() : CPPThreadRunner<int>()
		{
			Start();
		}
		/// <summary>
		/// Ctor allows setting a custom KeyboardPlayerInfo
		/// </summary>
		explicit KeyboardMapper(const sds::KeyboardPlayerInfo& player) : CPPThreadRunner<int>(), m_localPlayerInfo(player)
		{
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
		~KeyboardMapper() override
		{
			Stop();
		}
		bool IsControllerConnected() const
		{
			return m_poller.IsControllerConnected();
		}
		bool IsRunning() const
		{
			return m_poller.IsRunning() && this->m_is_thread_running;
		}
		void Start()
		{
			m_poller.Start();
			this->startThread();
		}
		void Stop()
		{
			m_poller.Stop();
			this->stopThread();
		}
		std::string AddMap(KeyboardKeyMap button)
		{
			if (IsRunning())
				Stop();
			std::string er = m_mapper.AddKeyMap(button);
			if (er.empty())
				Start();
			return er;
		}
	protected:
		/// <summary>
		/// Worker thread, private visibility, gets updated data from ProcessState() function to use.
		/// Accesses the std::atomic m_threadX and m_threadY members.
		/// </summary>
		void workThread() override
		{
			this->m_is_thread_running = true;
			//thread main loop
			while (!m_is_stop_requested)
			{
				const std::vector<XINPUT_KEYSTROKE> states = m_poller.GetUpdatedState();
				std::for_each(states.begin(), states.end(), [this](auto& cur)
					{
						m_mapper.ProcessKeystroke(cur);
					});
				//m_mapper.ProcessKeystroke(m_poller.GetUpdatedState());
				std::this_thread::sleep_for(std::chrono::milliseconds(KeyboardSettings::THREAD_DELAY_POLLER));
			}
			//mark thread status as not running.
			m_is_thread_running = false;
		}
	};
}

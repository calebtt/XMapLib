#pragma once
#include "stdafx.h"
#include "KeyboardTranslator.h"
#include "STRunner.h"
#include "STKeyboardPoller.h"

namespace sds
{
	/// <summary> This is an encapsulation function object with methods for accessing the
	///	data it is operating on. It has a member function operator()() called in the STRunner
	///	thread loop. This is so many objects such as this can be ran on the same thread.
	///	For performance reasons, typically the work thread in this functor will not be doing
	///	much beyond making a system call in a loop and updating something. Polling loops like
	///	this belong in a single thread without a delay to run them all.
	///	Remember that the m_is_enabled member of the base (DataWrapper) toggles on/off the processing of operator()()</summary>
	struct STKeyboardMapping : public sds::STRunner::DataWrapper
	{
	private:
		std::shared_ptr<STKeyboardPoller> m_poller;
		KeyboardTranslator m_translator{};
	public:
		virtual ~STKeyboardMapping() override
		{
			Stop();
		}
		explicit STKeyboardMapping(const std::shared_ptr<STKeyboardPoller> &poll, const LogFnType fn = nullptr)
		: DataWrapper(fn), m_poller(poll)
		{

		}
		/// <summary>Worker thread function called in a loop on the STRunner's thread.</summary>
		virtual void operator()() override
		{
			//TODO might want to use a mutex to make sure this isn't in some state of running for the start() stop() funcs
			//although might not be necessary.
			const std::vector<XINPUT_KEYSTROKE> states = m_poller->GetAndClearStates();
			for (const auto& cur : states)
			{
				m_translator.ProcessKeystroke(cur);
			}
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
		[[nodiscard]] std::vector<KeyboardKeyMap> GetMaps() const
		{
			return m_translator.GetMaps();
		}
		void ClearMaps()
		{
			m_translator.ClearMaps();
		}

		[[nodiscard]] bool IsRunning() const
		{
			return m_poller->IsEnabled() && m_is_enabled;
		}
		void Start() noexcept
		{
			m_poller->Start();
			m_is_enabled = true;
		}
		void Stop() noexcept
		{
			m_translator.CleanupInProgressEvents();
			m_is_enabled = false;
			m_poller->Stop();
		}
	};
}
#pragma once
#include "stdafx.h"
#include "KeyboardTranslator.h"
#include "STDataWrapper.h"

namespace sds
{
	/// <summary> It's a wrapper around KeyboardTranslator and KeyboardPoller that is added to the STRunner thread pool. Used in main class for use KeyboardMapper.
	/// Keyboard simulation function object that polls for controller input and processes it as keyboard keystrokes. It is ran on the STRunner thread pool.  </summary>
	///	<remarks>Remember that the m_is_enabled member of the base (STDataWrapper) toggles on/off the processing of operator()()</remarks>
	struct STKeyboardMapping : public STDataWrapper
	{
	private:
		// program settings pack for keyboard mapping.
		const KeyboardSettingsPack m_ksp;
		// class that contains the keypress handling logic.
		KeyboardTranslator m_translator;
		// class that wraps the syscalls for getting a controller update.
		KeyboardPoller m_poller;
	public:
		virtual ~STKeyboardMapping() override
		{
			Stop();
		}
		STKeyboardMapping(const KeyboardSettingsPack ksp = {},
			const LogFnType fn = nullptr
		)
		: STDataWrapper(fn),
		m_ksp(ksp),
		m_translator(ksp),
		m_poller(fn)
		{

		}
		/// <summary>Worker thread function called in a loop on the STRunner's thread.</summary>
		virtual void operator()() override
		{
			const auto stateUpdate = m_poller.GetUpdatedState(m_ksp.playerInfo.player_id);
			m_translator.ProcessKeystroke(stateUpdate);
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
			return m_is_enabled;
		}
		void Start() noexcept
		{
			m_is_enabled = true;
		}
		void Stop() noexcept
		{
			m_translator.CleanupInProgressEvents();
			m_is_enabled = false;
		}
	};
}

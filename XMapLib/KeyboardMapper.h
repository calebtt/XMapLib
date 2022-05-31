#pragma once
#include "stdafx.h"
#include "KeyboardTranslator.h"
#include "ControllerStatus.h"
#include "Utilities.h"
#include "STRunner.h"
#include "STKeyboardMapping.h"
#include "STKeyboardPoller.h"


namespace sds
{
	/// <summary>
	/// Main class for use, for mapping controller input to keyboard input.
	/// Uses KeyboardKeyMap for the details.
	/// </summary>
	class KeyboardMapper
	{
		using InternalType = int;
		using LambdaRunnerType = sds::CPPRunnerGeneric<InternalType>;
		using lock = LambdaRunnerType::ScopedLockType;

		std::shared_ptr<STKeyboardMapping> m_statMapping;
		std::shared_ptr<STKeyboardPoller> m_statPoller;
		STRunner m_statRunner;
		// Needed for iscontrollerconnected funcs
		const KeyboardPlayerInfo& m_localPlayerInfo;
	public:
		/// <summary>Ctor allows setting custom KeyboardPlayerInfo and KeyboardSettings</summary>
		explicit KeyboardMapper(const KeyboardPlayerInfo& player = {}, const KeyboardSettings& sett = {})
		: m_localPlayerInfo(player)
		{
			m_statPoller = std::make_shared<STKeyboardPoller>(player, sett, Utilities::LogError);
			m_statMapping = std::make_shared<STKeyboardMapping>(m_statPoller, Utilities::LogError);
			m_statRunner.AddDataWrapper(m_statPoller);
			m_statRunner.AddDataWrapper(m_statMapping);
		}
		// Other constructors/destructors
		KeyboardMapper(const KeyboardMapper& other) = delete;
		KeyboardMapper(KeyboardMapper&& other) = delete;
		KeyboardMapper& operator=(const KeyboardMapper& other) = delete;
		KeyboardMapper& operator=(KeyboardMapper&& other) = delete;
		~KeyboardMapper() = default;

		[[nodiscard]] bool IsControllerConnected() const
		{
			return ControllerStatus::IsControllerConnected(m_localPlayerInfo.player_id);
		}
		[[nodiscard]] bool IsRunning() const
		{
			return m_statMapping->IsRunning() && m_statRunner.IsRunning();
		}
		std::string AddMap(KeyboardKeyMap button) const
		{
			return m_statMapping->AddMap(button);
		}
		[[nodiscard]] std::vector<KeyboardKeyMap> GetMaps() const
		{
			return m_statMapping->GetMaps();
		}
		void ClearMaps() const
		{
			m_statMapping->ClearMaps();
		}
		void Start() noexcept
		{
			m_statMapping->Start();
			m_statRunner.StartThread();
		}
		void Stop() noexcept
		{
			m_statMapping->Stop();
			m_statRunner.StopThread();
		}
	};
}

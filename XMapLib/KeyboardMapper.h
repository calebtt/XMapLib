#pragma once
#include "stdafx.h"
#include "KeyboardTranslator.h"
#include "ControllerStatus.h"
#include "Utilities.h"
#include "STRunner.h"
#include "STKeyboardMapping.h"
#include "Smarts.h"

namespace sds
{
	/// <summary>
	/// Main class for use, for mapping controller input to keyboard input.
	/// Uses KeyboardKeyMap for the details.
	/// </summary>
	template<class LogFnType = std::function<void(std::string)>>
	class KeyboardMapperImpl
	{
		// Static thread runner (single thread of a thread pool).
		std::shared_ptr<STRunnerImpl<LogFnType>> m_statRunner;
		// Keyboard settings pack, needed for iscontrollerconnected func arcs and others.
		const KeyboardSettingsPack m_keySettingsPack;
		// Logging function, optionally passed in by the user.
		const LogFnType m_logFn;
		// Combined object for polling and translation into action,
		// to be ran on an STRunner object.
		SharedPtrType<STKeyboardMappingImpl<LogFnType>> m_statMapping;
	public:
		/// <summary>Ctor allows passing in a STRunner thread, and setting custom KeyboardPlayerInfo and KeyboardSettings
		/// with optional logging function. </summary>
		KeyboardMapperImpl( const SharedPtrType<STRunnerImpl<LogFnType>> &statRunner,
			const KeyboardSettingsPack &settPack = {},
			const LogFnType logFn = nullptr )
			: m_statRunner(std::move(statRunner)),
			m_keySettingsPack(settPack),
			m_logFn(logFn)
		{
			// lambda for logging
			auto LogIfAvailable = [&](const char* msg)
			{
				if (m_logFn != nullptr)
					m_logFn(msg);
			};
			// if statRunner is nullptr, log error and return
			if (m_statRunner == nullptr)
			{
				LogIfAvailable("Exception: In KeyboardMapper::KeyboardMapper(...): STRunner shared_ptr was null!");
				return;
			}
			// otherwise, add the keyboard mapping obj to the STRunner thread for processing
			m_statMapping = MakeSharedSmart<STKeyboardMappingImpl<LogFnType>>(m_keySettingsPack, m_logFn);
			m_statRunner->AddDataWrapper(m_statMapping);
		}
		// Other constructors/destructors
		KeyboardMapperImpl(const KeyboardMapperImpl& other) = delete;
		KeyboardMapperImpl(KeyboardMapperImpl&& other) = delete;
		KeyboardMapperImpl& operator=(const KeyboardMapperImpl& other) = delete;
		KeyboardMapperImpl& operator=(KeyboardMapperImpl&& other) = delete;
		~KeyboardMapperImpl() = default;

		[[nodiscard]] bool IsControllerConnected() const
		{
			return ControllerStatus::IsControllerConnected(m_keySettingsPack.playerInfo.player_id);
		}
		/// <summary> Called to query if this instance is enabled and the thread pool thread is running. </summary>
		///	<returns> returns true if both are running, false on error </returns>
		[[nodiscard]] bool IsRunning() const
		{
			if (m_statRunner == nullptr || m_statMapping == nullptr)
				return false;
			return m_statMapping->IsRunning() && m_statRunner->IsRunning();
		}
		/// <summary><c>AddMap(KeyboardKeyMap)</c> Adds a key map.</summary>
		///	<returns> returns a <c>std::string</c> containing an error message
		///	if there is an error, empty string otherwise. </returns>
		std::string AddMap(KeyboardKeyMap button) const
		{
			if (m_statMapping == nullptr)
				return "Exception in KeyboardMapper::AddMap(...): m_statMapping is null.";
			return m_statMapping->AddMap(button);
		}
		[[nodiscard]] std::vector<KeyboardKeyMap> GetMaps() const
		{
			if (m_statMapping == nullptr)
				return {};
			return m_statMapping->GetMaps();
		}
		void ClearMaps() const
		{
			if(m_statMapping != nullptr)
				m_statMapping->ClearMaps();
		}
		/// <summary> Enables processing on the function objects added to the STRunner thread pool.
		/// Does not start the STRunner thread! </summary>
		void Start() noexcept
		{
			if(m_statMapping != nullptr)
				m_statMapping->Start();
		}
		/// <summary> Disables processing on the function objects added to the STRunner thread pool.
		///	Does not stop the STRunner thread! </summary>
		void Stop() const noexcept
		{
			if(m_statMapping != nullptr)
				m_statMapping->Stop();
		}
	};

	// Using declaration for standard config.
	using KeyboardMapper = KeyboardMapperImpl<>;
}

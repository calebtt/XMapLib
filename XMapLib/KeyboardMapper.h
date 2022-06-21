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
	class KeyboardMapper
	{
		// Alias for logging function pointer type.
		using LogFnType = std::function<void(std::string)>;
		// Static thread runner (single thread of a thread pool).
		std::shared_ptr<STRunner> m_statRunner;
		// Keyboard settings pack, needed for iscontrollerconnected func arcs and others.
		const KeyboardSettingsPack m_keySettingsPack;
		// Logging function, optionally passed in by the user.
		const LogFnType m_logFn;
		// Combined object for polling and translation into action,
		// to be ran on an STRunner object.
		SharedPtrType<STKeyboardMapping> m_statMapping;
	public:
		/// <summary>Ctor allows passing in a STRunner thread, and setting custom KeyboardPlayerInfo and KeyboardSettings
		/// with optional logging function. </summary>
		KeyboardMapper( SharedPtrType<STRunner> statRunner,
			const KeyboardSettingsPack settPack = {},
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
			m_statMapping = MakeSharedSmart<STKeyboardMapping>(m_keySettingsPack, m_logFn);
			m_statRunner->AddDataWrapper(m_statMapping);
		}
		// Other constructors/destructors
		KeyboardMapper(const KeyboardMapper& other) = delete;
		KeyboardMapper(KeyboardMapper&& other) = delete;
		KeyboardMapper& operator=(const KeyboardMapper& other) = delete;
		KeyboardMapper& operator=(KeyboardMapper&& other) = delete;
		~KeyboardMapper() = default;

		[[nodiscard]] bool IsControllerConnected() const
		{
			return ControllerStatus::IsControllerConnected(m_keySettingsPack.playerInfo.player_id);
		}
		[[nodiscard]] bool IsRunning() const
		{
			return m_statMapping->IsRunning() && m_statRunner->IsRunning();
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
		/// <summary> Enables processing on the function objects added to the STRunner thread pool.
		/// Does not start the STRunner thread! </summary>
		void Start() noexcept
		{
			m_statMapping->Start();
		}
		/// <summary> Disables processing on the function objects added to the STRunner thread pool.
		///	Does not stop the STRunner thread! </summary>
		void Stop() const noexcept
		{
			m_statMapping->Stop();
		}
	};
}

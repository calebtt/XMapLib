#pragma once
#include "stdafx.h"
#include "KeyboardTranslator.h"
#include "ControllerStatus.h"
#include "Utilities.h"
#include "STRunner.h"
#include "STKeyboardMapping.h"

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
		std::shared_ptr<STKeyboardMapping> m_statMapping;
	public:
		/// <summary>Ctor allows setting custom KeyboardPlayerInfo and KeyboardSettings</summary>
		KeyboardMapper(const std::shared_ptr<STRunner> &statRunner = nullptr,
			const KeyboardSettingsPack settPack = {},
			const LogFnType logFn = nullptr
		)
		: m_statRunner(statRunner),
		m_keySettingsPack(settPack),
		m_logFn(logFn)
		{
			auto LogIfAvailable = [&](const char* msg)
			{
				if (m_logFn != nullptr)
					m_logFn(msg);
			};
			if (m_statRunner == nullptr)
			{
				LogIfAvailable("Information: In KeyboardMapper::KeyboardMapper(...): STRunner shared_ptr was null, creating a new instance.");
				m_statRunner = std::make_shared<STRunner>(logFn);
			}
			m_statMapping = std::make_shared<STKeyboardMapping>(m_keySettingsPack, m_logFn);
			if (!m_statRunner->IsRunning())
			{
				LogIfAvailable("Information: In KeyboardMapper::KeyboardMapper(...): STRunner was not already running, starting thread...");
				if (!m_statRunner->StartThread())
					LogIfAvailable("Exception: In KeyboardMapper::KeyboardMapper(...): STRunner reported it was not able to start the thread!");
			}
			if(!m_statRunner->AddDataWrapper(m_statMapping))
			{
				LogIfAvailable("Exception: In KeyboardMapper::KeyboardMapper(...): STRunner reported it was not able to add the wrapper!");
			}
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
		/// Does not start the STRunner thread! Will add the necessary function objects to the STRunner processing vector
		/// when called, if not present. </summary>
		void Start() noexcept
		{
			m_statMapping->Start();
			const auto fnList = m_statRunner->GetWrapperBuffer();
			const auto tempIt = std::find_if(fnList.cbegin(), fnList.cend(), [&](const auto& elem)
				{
					return elem.get() == m_statMapping.get();
				});
			//if shared_ptr to our mapping object was not found in the functor list, add it
			if(tempIt == fnList.end())
			{
				m_statRunner->AddDataWrapper(m_statMapping);
			}
		}
		/// <summary> Disables processing on the function objects added to the STRunner thread pool.
		///	Does not stop the STRunner thread! </summary>
		void Stop() const noexcept
		{
			m_statMapping->Stop();
		}
	};
}

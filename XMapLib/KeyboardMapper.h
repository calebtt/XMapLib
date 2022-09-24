#pragma once
#include "stdafx.h"
#include "KeyboardTranslator.h"
#include "ControllerStatus.h"
#include "Utilities.h"
#include "STKeyboardMapping.h"
#include "Smarts.h"
#include "../impcool_sol/immutable_thread_pool/ThreadPool.h"
#include "../impcool_sol/immutable_thread_pool/ThreadUnitPlus.h"

namespace sds
{
	/// <summary>
	/// Main class for use, for mapping controller input to keyboard input.
	/// Uses KeyboardKeyMap for the details.
	///	<para> Construction requires an instance of a <c>ThreadUnitPlus</c> type, managing
	///	infinite tasks running on a single thread. </para>
	///	</summary>
	template<class LogFnType = std::function<void(std::string)>>
	class KeyboardMapper
	{
	public:
		using ThreadManager = impcool::ThreadUnitPlus;
	private:
		// Thread unit, runs tasks.
		SharedPtrType<ThreadManager> m_statRunner;
		// Keyboard settings pack, needed for iscontrollerconnected func arcs and others.
		const KeyboardSettingsPack m_keySettingsPack;
		// Logging function, optionally passed in by the user.
		const LogFnType m_logFn;
		// Combined object for polling and translation into action,
		// to be ran on an STRunner object.
		SharedPtrType<STKeyboardMapping<LogFnType>> m_statMapping;

	public:
		/// <summary>Ctor allows passing in a STRunner thread, and setting custom KeyboardPlayerInfo and KeyboardSettings
		/// with optional logging function. </summary>
		KeyboardMapper( const SharedPtrType<ThreadManager> &statRunner, const KeyboardSettingsPack settPack = {}, const LogFnType logFn = nullptr )
			: m_statRunner(statRunner),
			m_keySettingsPack(settPack),
			m_logFn(logFn)
		{
			// TODO decide whether the logging fn is worth keeping around or not.
			// lambda for logging
			auto LogIfAvailable = [&](const char* msg)
			{
				if (m_logFn != nullptr)
					m_logFn(msg);
				else
					throw std::exception(msg);
			};
			// if statRunner is nullptr, log error and return
			if (m_statRunner == nullptr)
			{
				LogIfAvailable("Exception: In KeyboardMapper::KeyboardMapper(...): statRunner shared_ptr was null!");
				return;
			}
			// otherwise, add the keyboard mapping obj to the STRunner thread for processing
			m_statMapping = MakeSharedSmart<STKeyboardMapping<LogFnType>>(m_keySettingsPack, m_logFn);
			auto tempMapping = m_statMapping;
			m_statRunner->PushInfiniteTaskBack([tempMapping]() { tempMapping->operator()(); });
		}
		// Other constructors/destructors
		KeyboardMapper(const KeyboardMapper& other) = delete;
		KeyboardMapper(KeyboardMapper&& other) = delete;
		KeyboardMapper& operator=(const KeyboardMapper& other) = delete;
		KeyboardMapper& operator=(KeyboardMapper&& other) = delete;
		~KeyboardMapper()
		{
			m_statRunner->DestroyThread();
		}

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
			return m_statMapping->IsRunning();
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
}

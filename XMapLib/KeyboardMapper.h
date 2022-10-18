#pragma once
#include "stdafx.h"
#include "KeyboardTranslator.h"
#include "ControllerStatus.h"
#include "KeyboardTranslatorAsync.h"
#include "Utilities.h"
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
	class KeyboardMapper
	{
		/// <summary>
		/// Struct holding an input polling obj and a keypress simulation translator obj,
		/// with an atomic to disable processing. Also contains the work thread added to the
		///	thread pool.
		/// </summary>
		///	<remarks> <b>NOTE</b>: An instance of this is stored in a shared_ptr that is copied
		///	into the lambda! Not captured by reference, this is intentional to allow the std::function
		///	type-erasure aspect of the object to extend the lifetime of the shared_ptr data member of the lambda as it requires! </remarks>
		struct PollingAndTranslation
		{
			// class that contains the keypress handling logic, used async.
			KeyboardTranslatorAsync m_translator;
			// class that wraps the syscalls for getting a controller update.
			KeyboardPoller m_poller;
			// bool to disable processing
			std::atomic<bool> m_is_enabled{ true };
			// Main loop fn (Runs on another thread)
			void DoWork(const int playerId) noexcept
			{
				if (m_is_enabled)
				{
					const auto stateUpdate = m_poller.GetUpdatedState(playerId);
					m_translator.ProcessKeystroke(stateUpdate);
				}
			}
		};
	public:
		using ThreadManager = impcool::ThreadUnitPlus;
	private:
		// Thread unit, runs tasks.
		SharedPtrType<ThreadManager> m_statRunner;
		// Keyboard settings pack, needed for iscontrollerconnected func args and others.
		KeyboardSettingsPack m_keySettingsPack;
		// Combined object for polling and translation into action,
		// to be ran on a thread pool type object.
		SharedPtrType<PollingAndTranslation> m_statMapping;
	public:
		/// <summary>Ctor allows passing in a STRunner thread, and setting custom KeyboardPlayerInfo and KeyboardSettings
		/// with optional logging function. </summary>
		KeyboardMapper( const SharedPtrType<ThreadManager> &statRunner, const KeyboardSettingsPack settPack = {})
			: m_statRunner(statRunner),
			m_keySettingsPack(settPack)
		{
			// if statRunner is nullptr, report error
			assert(m_statRunner != nullptr);
			// Add the keyboard polling and translation function to the thread for processing
			m_statMapping = MakeSharedSmart<PollingAndTranslation>();
			std::shared_ptr<PollingAndTranslation> tempMapping = m_statMapping;
			// lambda to push into the task list
			const int pid = m_keySettingsPack.playerInfo.player_id;
			const auto taskLam = [tempMapping, pid]() { tempMapping->DoWork(pid); };
			m_statRunner->PushInfiniteTaskBack(taskLam);
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
			return m_statMapping->m_is_enabled;
		}
		/// <summary><c>AddMap(KeyboardKeyMap)</c> Adds a key map.</summary>
		///	<returns> returns a <c>std::string</c> containing an error message
		///	if there is an error, empty string otherwise. </returns>
		std::string AddMap(KeyboardKeyMap button) const
		{
			if (m_statMapping == nullptr)
				return "Exception in KeyboardMapper::AddMap(...): m_statMapping is null.";
			return m_statMapping->m_translator.AddKeyMap(button);
		}
		[[nodiscard]] std::vector<KeyboardKeyMap> GetMaps() const
		{
			if (m_statMapping == nullptr)
				return {};
			return m_statMapping->m_translator.GetMaps();
		}
		void ClearMaps() const noexcept
		{
			if (m_statMapping != nullptr)
				m_statMapping->m_translator.ClearMaps();
		}
		/// <summary> Enables processing on the function(s) added to the thread pool.
		/// Does not start the pool thread! </summary>
		void Start() const noexcept
		{
			if (m_statMapping != nullptr)
				m_statMapping->m_is_enabled = true;
		}
		/// <summary> Disables processing on the function(s) added to the thread pool.
		///	Does not stop the pool thread! </summary>
		void Stop() const noexcept
		{
			if (m_statMapping != nullptr)
				m_statMapping->m_is_enabled = false;
		}
	};
}

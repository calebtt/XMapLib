#pragma once
#include "stdafx.h"
#include "KeyboardTranslator.h"
#include "ControllerStatus.h"
#include "ControllerButtonToActionMap.h"
#include "Utilities.h"
#include "Smarts.h"
#include "../impcool_sol/immutable_thread_pool/ThreadPooler.h"
#include "../impcool_sol/immutable_thread_pool/ThreadUnitPlusPlus.h"
#include "../impcool_sol/immutable_thread_pool/ThreadTaskSource.h"

namespace sds
{
	template<typename Poller_t>
	concept IsInputPoller = requires(Poller_t & t)
	{
		{ t.GetUpdatedState(0) };
		{ t.GetUpdatedState(0) } -> std::convertible_to<KeyStateWrapper>;
	};

	/// <summary> Main class for use, for mapping controller input to keyboard input. Uses ControllerButtonToActionMap for the details.
	///	<para> Construction requires an instance of a <c>ThreadUnitPlus</c> type, managing infinite tasks running on a single thread. </para>
	///	<para> Copyable, movable. Shallow copy of shared_ptr to polling and translation. </para>
	///	</summary>
	template<IsInputPoller InputPoller_t = KeyboardPoller, typename ThreadPool_t = imp::ThreadUnitPlusPlus>
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
			InputPoller_t m_poller;
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
		using ThreadManager = ThreadPool_t;
		using KeyMapRange_t = std::deque<ControllerButtonToActionMap>;
	private:
		// Thread unit, runs tasks.
		SharedPtrType<ThreadManager> m_statRunner;
		// Keyboard settings pack, needed for iscontrollerconnected func args and others.
		KeyboardSettingsPack m_keySettingsPack;
		// Combined object for polling and translation into action,
		// to be ran on a thread pool type object.
		SharedPtrType<PollingAndTranslation> m_statMapping;
		// Range holding our key maps.
		KeyMapRange_t m_keyMaps;
	public:
		/// <summary>Ctor allows passing in a STRunner thread, and setting custom KeyboardPlayerInfo and KeyboardSettings
		/// with optional logging function. </summary>
		KeyboardMapper( 
			const SharedPtrType<ThreadManager> &statRunner, 
			const KeyboardSettingsPack &settPack = {})
			: m_statRunner(statRunner),
			m_keySettingsPack(settPack)
		{
			// if statRunner is nullptr, report error
			assert(m_statRunner != nullptr);
			// Construct a keyboard polling and translation object, will be used on the thread.
			std::shared_ptr<PollingAndTranslation> tempMapping = MakeSharedSmart<PollingAndTranslation>();
			// Assign data member to control the lifetime here and to access the member functions.
			m_statMapping = tempMapping;
			// Get existing task source, push additional task.
			auto tempSource = statRunner->GetTaskSource();
			// lambda to push into the task list
			const int pid = m_keySettingsPack.playerInfo.player_id;
			const auto taskLam = [tempMapping, pid]() { tempMapping->DoWork(pid); };
			tempSource.PushInfiniteTaskBack(taskLam);
			// Finally, set the task source.
			m_statRunner->SetTaskSource(tempSource);
		}
		// Other constructors/destructors
		KeyboardMapper(const KeyboardMapper& other) = default;
		KeyboardMapper(KeyboardMapper&& other) = default;
		KeyboardMapper& operator=(const KeyboardMapper& other) = default;
		KeyboardMapper& operator=(KeyboardMapper&& other) = default;
		~KeyboardMapper() = default;

		[[nodiscard]]
		auto IsControllerConnected() const -> bool
		{
			return ControllerStatus::IsControllerConnected(m_keySettingsPack.playerInfo.player_id);
		}
		/// <summary> Called to query if this instance is enabled and the thread pool thread is running. </summary>
		///	<returns> returns true if both are running, false on error </returns>
		[[nodiscard]]
		auto IsRunning() const -> bool
		{
			if (m_statRunner == nullptr || m_statMapping == nullptr)
				return false;
			return m_statMapping->m_is_enabled;
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
	public:
		[[nodiscard]]
		auto GetMaps() const noexcept -> KeyMapRange_t
		{
			return m_keyMaps;
		}
		// Call with no arg to clear the key maps.
		void SetMaps(const KeyMapRange_t &keys = {})
		{
			m_keyMaps = {};
			for (const auto& elem : keys)
			{
				m_keyMaps.emplace_back(elem);
			}
		}
	};
}

#pragma once
#include "stdafx.h"
#include <atomic>
#include "ControllerStatus.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardMapSource.h"
#include "KeyboardPoller.h"
#include "Utilities.h"
#include "Smarts.h"
#include "../impcool_sol/immutable_thread_pool/pausable_async.h"

namespace sds
{
	//template<typename Poller_t>
	//concept IsInputPoller = requires(Poller_t & t)
	//{
	//	{ t.GetUpdatedState(0) };
	//	{ t.GetUpdatedState(0) } -> std::convertible_to<ControllerStateWrapper>;
	//};

	/// <summary> Main class for use, for mapping controller input to keyboard input. Uses ControllerButtonToActionMap for the details.
	///	<para> Construction requires an instance of a <c>ThreadUnitPlus</c> type, managing infinite tasks running on a single thread. </para>
	///	<para> Copyable, movable. Shallow copy of shared_ptr to polling and translation. </para>
	///	</summary>
	template<typename InputPoller_t = KeyboardPoller>
	class KeyboardMapper
	{
	private:
		InputPoller_t m_poller;
		KeyboardSettingsPack m_keySettingsPack;
		KeyboardMapSource m_mappings;

		std::atomic<bool> m_stopReq{ false };
		std::atomic<bool> completionNotifier{ false };
		std::future<void> tickFuture;
	public:
		/// <summary>Ctor allows passing in a STRunner thread, and setting custom KeyboardPlayerInfo and KeyboardSettings
		/// with optional logging function. </summary>
		KeyboardMapper(const KeyboardSettingsPack &settPack = {})
			: m_keySettingsPack(settPack)
		{
			StartThreadFn();
		}
		// Other constructors/destructors
		KeyboardMapper(const KeyboardMapper& other) = default;
		KeyboardMapper(KeyboardMapper&& other) = default;
		KeyboardMapper& operator=(const KeyboardMapper& other) = default;
		KeyboardMapper& operator=(KeyboardMapper&& other) = default;
		~KeyboardMapper() = default;

		auto StartThreadFn()
		{
			// TODO this can work like this for now, but eventually will be controlled by
			// the "tick" timing of a process() loop.
			auto ProcessFn = [&]()
			{
				m_mappings.ProcessState(m_poller.GetUpdatedState(m_keySettingsPack.playerInfo.player_id));
			};
			const auto TickFn = [&]()
			{
				while (!m_stopReq.stop_requested())
				{
					m_mappings.ProcessState(m_poller.GetUpdatedState(m_keySettingsPack.playerInfo.player_id));
				}
			};
			tickFuture = std::async(std::launch::async, TickFn);
		}

		[[nodiscard]]
		auto IsControllerConnected() const -> bool
		{
			return ControllerStatus::IsControllerConnected(m_keySettingsPack.playerInfo.player_id);
		}
		///// <summary> Called to query if this instance is enabled and the thread pool thread is running. </summary>
		/////	<returns> returns true if both are running, false on error </returns>
		//[[nodiscard]]
		//auto IsRunning() const -> bool
		//{
		//	if (m_statRunner == nullptr || m_statMapping == nullptr)
		//		return false;
		//	return m_statMapping->m_is_enabled;
		//}

		void Start() noexcept
		{
			tickFuture.wait();
			m_stopReq = false;
			StartThreadFn();
		}

		void Stop() noexcept
		{
			m_stopReq = true;
		}
	public:
		[[nodiscard]]
		auto GetMaps() const noexcept -> KeyboardMapSource
		{
			return m_mappings;
		}
		// Call with no arg to clear the key maps.
		void SetMaps(const KeyboardMapSource &keys = {})
		{
			m_mappings.ClearMaps();
			m_mappings = keys;
		}
	};
}

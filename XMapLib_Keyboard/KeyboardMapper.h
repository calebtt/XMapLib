#pragma once
#include "LibIncludes.h"
#include <atomic>
#include "ControllerButtonToActionMap.h"
#include "KeyboardMapSource.h"
#include "KeyboardPoller.h"
#include "KeyboardTranslator.h"

namespace sds
{
	template<typename Poller_t>
	concept IsInputPoller = requires(Poller_t & t)
	{
		{ t.GetUpdatedState(0) };
		{ t.GetUpdatedState(0) } -> std::convertible_to<ControllerStateWrapper>;
	};
	template<typename T>
	concept IsRv = requires(T & t)
	{
		{ !std::is_lvalue_reference_v<T> };
	};
	/**
	 * \brief Main class for use, for mapping controller input to <b>keyboard</b> input.
	 * Construction requires an instance of a <b>Thread Unit</b> type, managing infinite tasks running on a single thread.
	 * Copyable, movable. Shallow copy of shared_ptr to polling and translation.
	 * \tparam InputPoller_t Type used for polling for controller updates.
	 */
	template<typename ThreadUnit_t = imp::ThreadUnitFP,
		IsInputPoller InputPoller_t = KeyboardPoller>
	class KeyboardMapper
	{
	private:
		// Thread task pool class, our work functors get added to here and called in succession on a separate thread for performance reasons.
		SharedPtrType<ThreadUnit_t> m_statRunner;
		SharedPtrType<InputPoller_t> m_poller;
		KeyboardSettingsPack m_keySettingsPack;
		CBActionTranslator m_translator;
		//SharedPtrType<KeyboardMapSource> m_mappings = MakeSharedSmart<KeyboardMapSource>();
		std::atomic<bool> m_stopReq{ false };
	public:
		/**
		 * \brief 
		 * \param statRunner The thread to run it all on.
		 * \param poller The source for updated controller state information.
		 * \param translator std::move'd in, The object that produces updates wrt the mappings.
		 * \param settPack Keyboard settings pack
		 */
		KeyboardMapper( 
			const SharedPtrType<ThreadUnit_t>& statRunner,
			const SharedPtrType<InputPoller_t>& poller,
			CBActionTranslator&& translator,
			const KeyboardSettingsPack &settPack = {})
		:
		m_statRunner(statRunner),
		m_poller(poller),
		m_translator(std::move(translator)),
		m_keySettingsPack(settPack)
		{
			assert(m_statRunner != nullptr);
			auto taskContainer = m_statRunner->GetTaskSource();
			taskContainer.PushInfiniteTaskBack([this]()
			{
				threadFunc();
			});
			m_statRunner->SetTaskSource(taskContainer);
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

		void Start() noexcept
		{
			m_stopReq = false;
		}

		void Stop() noexcept
		{
			m_stopReq = true;
		}
	public:
		//[[nodiscard]]
		//auto GetMaps() const noexcept -> KeyboardMapSource
		//{
		//	return *m_mappings;
		//}
		// Call with no arg to clear the key maps.
		//void SetMaps(const KeyboardMapSource &keys = {}) const noexcept
		//{
		//	m_mappings->ClearMaps();
		//	*m_mappings = keys;
		//}
	private:
		auto threadFunc() 
		{
			if (!m_stopReq)
			{
				//m_mappings->ProcessState(m_poller->GetUpdatedState(m_keySettingsPack.playerInfo.player_id));
			}
		}
	};
}

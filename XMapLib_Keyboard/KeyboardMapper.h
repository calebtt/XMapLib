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

	/**
	 * \brief Top-level object, callable object, returns a vector of TranslationResult.
	 * Construction requires a shared_ptr to InputPoller type--the source for controller state info.
	 * Construction also requires a pre-constructed Translator type--
	 * Copyable, movable. Shallow copy of shared_ptr to polling and translation.
	 * \tparam InputPoller_t Type used for polling for controller updates. Class for polling on other platforms can be substituted here.
	 * \tparam Translator_t Type for the state change interpretation logic, might be upgraded someday and is thus a template param.
	 */
	template<IsInputPoller InputPoller_t = KeyboardPoller, typename Translator_t = CBActionTranslator>
	class KeyboardMapper
	{
	private:
		SharedPtrType<InputPoller_t> m_poller;
		//SharedPtrType<Translator_t> m_translator;
	public:
		/**
		 * \brief Public data member, encapsulates the maps.
		 */
		Translator_t Translator;
	public:
		/**
		 * \brief 
		 * \param poller The source for updated controller state information.
		 * \param translator std::move'd in, The object that produces updates wrt the mappings.
		 */
		KeyboardMapper( 
			const SharedPtrType<InputPoller_t>& poller,
			CBActionTranslator&& translator)
		: m_poller(poller), Translator(std::move(translator))
		{
			assert(m_poller != nullptr);
			//// Add task to the container.
			//auto taskContainer = m_statRunner->GetTaskSource();
			//taskContainer.PushInfiniteTaskBack([this]()
			//{
			//	threadFunc();
			//});
			//m_statRunner->SetTaskSource(taskContainer);
		}
		// Other constructors/destructors
		KeyboardMapper(const KeyboardMapper& other) = default;
		KeyboardMapper(KeyboardMapper&& other) = default;
		KeyboardMapper& operator=(const KeyboardMapper& other) = default;
		KeyboardMapper& operator=(KeyboardMapper&& other) = default;
		~KeyboardMapper() = default;
	public:
		/**
		 * \brief Runs synchronously.
		 * \return Returns vector of translation results.
		 */
		auto GetUpdate()
		{
			return Translator.ProcessState(m_poller->GetUpdatedState());
		}

		/**
		 * \brief Runs synchronously.
		 * \return Returns accumulated vector of translation results from an array of many polled states.
		 */
		auto GetChunkUpdates()
		{
			std::vector<TranslationResult> actions;
			const auto stateList = m_poller->GetUpdatedStateQueue();
			for(const auto& elem: stateList)
			{
				actions.append_range(Translator.ProcessState(elem));
			}
			return actions;
		}
	};
}

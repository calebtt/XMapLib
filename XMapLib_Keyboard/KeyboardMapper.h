#pragma once
#include "LibIncludes.h"
#include <atomic>
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include "KeyboardActionTranslator.h"

namespace sds
{
	template<typename Poller_t>
	concept IsInputPoller = requires(Poller_t & t)
	{
		{ t.GetUpdatedState(0) };
		{ t.GetUpdatedState(0) } -> std::convertible_to<ControllerStateWrapper>;
	};

	/**
	 * \brief Function used to "perform" the action suggested by the TranslationResult and then update the
	 * mapping object's "LastState" member.
	 * \remarks Calls the OnEvent function, and then updates the state machine.
	 * \param tr The object upon which to perform the action.
	 */
	inline
	auto CallAndUpdateTranslationResult(sds::TranslationResult& tr) -> void
	{
		// Don't need to test for containing a function, they will--just might not do anything.
		tr.OperationToPerform();
		tr.AdvanceStateFn();
	}

	// TODO this either won't be used or will just be a facade holding the top-level objects actually in-use.
	/**
	 * \brief Top-level object, callable object, returns a vector of TranslationResult.
	 * Construction requires a shared_ptr to InputPoller type--the source for controller state info.
	 * Construction also requires a pre-constructed Translator type--
	 * Copyable, movable. Shallow copy of shared_ptr to polling and translation.
	 * \tparam InputPoller_t Type used for polling for controller updates. Class for polling on other platforms can be substituted here.
	 * \tparam Translator_t Type for the state change interpretation logic, might be upgraded someday and is thus a template param.
	 */
	//template<IsInputPoller InputPoller_t = KeyboardPoller, typename Translator_t = KeyboardActionTranslator>
	//class KeyboardMapper
	//{
	//private:
	//	SharedPtrType<InputPoller_t> m_poller;
	//	//SharedPtrType<Translator_t> m_translator;
	//public:
	//	/**
	//	 * \brief Public data member, encapsulates the maps.
	//	 */
	//	Translator_t Translator;
	//public:
	//	/**
	//	 * \brief Requires a translator object which encapsulates some mappings.
	//	 * \param translator std::move'd in, The object that produces updates wrt the mappings.
	//	 */
	//	KeyboardMapper( KeyboardActionTranslator&& translator) : Translator(std::move(translator)) { }
	//	// Other constructors/destructors
	//	KeyboardMapper(const KeyboardMapper& other) = default;
	//	KeyboardMapper(KeyboardMapper&& other) = default;
	//	KeyboardMapper& operator=(const KeyboardMapper& other) = default;
	//	KeyboardMapper& operator=(KeyboardMapper&& other) = default;
	//	~KeyboardMapper() = default;
	//public:
	//	/**
	//	 * \brief Runs synchronously.
	//	 * \return Returns vector of translation results.
	//	 */
	//	auto GetUpdate(auto& poller)
	//	{
	//		return Translator.ProcessState(poller());
	//	}

	//	/**
	//	 * \brief Runs synchronously.
	//	 * \return Returns accumulated vector of translation results from an array of many polled states.
	//	 */
	//	auto GetChunkUpdates()
	//	{
	//		std::vector<TranslationResult> actions;
	//		const auto stateList = m_poller->GetUpdatedStateQueue();
	//		for(const auto& elem: stateList)
	//		{
	//			actions.append_range(Translator.ProcessState(elem));
	//		}
	//		return actions;
	//	}
	//};
}

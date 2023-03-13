#pragma once
#include "LibIncludes.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include <iostream>
#include <chrono>
#include <optional>
#include <vector>
#include <iterator>
#include <tuple>
#include <ranges>

namespace sds
{
	// TODO utilize this, for to a real pipeline make
	// It will be the return value of a call to the Translator's "process" function, and will be returned back up
	// to the top-level to be passed to another object for performing the action based on it.
	struct TranslationResult
	{
		bool DoDown{ false };
		bool DoUp{ false };
		bool DoRepeat{ false };
		bool DoReset{ false };
		CBActionMap* ButtonMapping;
	};
	//TODO if this were a module, this would probably not be exported.
	using MappingAndStatePair_t = std::pair<CBActionMap, MappingStateManager>;

	template<typename T>
	concept MappingRange_c = requires (T & t)
	{
		{ std::ranges::range<T> };
		{ std::same_as<typename T::value_type, CBActionMap> };
	};
	/*
	 *	Free functions.
	 */

	/**
	 * \brief Checks a list of mappings for having multiple exclusivity groupings mapped to a single controller button.
	 * \param mappingsList List of controller button to action mappings.
	 * \return true if good mapping list, false if there is a problem.
	 */
	[[nodiscard]]
	inline
	bool AreExclusivityGroupsUnique(const std::vector<MappingAndStatePair_t>& mappingsList)
	{
		std::map<int, std::optional<int>> groupMap;
		for (const auto& e : mappingsList)
		{
			// If an exclusivity group is set, we must verify no duplicate ex groups are set to the same vk
			const auto& vk = e.first.Vk;
			const auto& currentMappingGroupOpt = e.first.ExclusivityGrouping;
			const auto& existingGroupOpt = groupMap[vk];
			if (currentMappingGroupOpt.has_value() && existingGroupOpt.has_value())
			{
				if (existingGroupOpt.value() != currentMappingGroupOpt.value())
					return false;
			}
			groupMap[vk] = currentMappingGroupOpt;
		}
		return true;
	}

	// Note, only returns ex. group matches that don't map to the same controller button.
	inline
	auto GetMatchingExclusivityGroupMappings(CBActionMap& mapping, MappingRange_c auto& mappingsList)
	{
		using MappingsList_t = decltype(mappingsList);
		using MappingsListIterator_t = typename MappingsList_t::iterator;
		using GrpVal_t = typename decltype(mapping)::ExclusivityGrouping::value_type;
		// A requirement as we compare -1 and 0 in event of (somehow) empty optional, fix if necessary.
		static_assert(std::is_same_v<GrpVal_t, int>());
		using std::ranges::begin, std::ranges::end, std::ranges::for_each, std::ranges::cbegin, std::ranges::cend;

		// Create vec of pointers and add elements matching the VK
		std::vector<MappingsListIterator_t> iterBuffer;
		for_each(mappingsList, [&](MappingsListIterator_t elem)
		{
			const auto& [fst, snd] = *elem;
			const auto& mappingGroup = mapping.ExclusivityGrouping;
			const auto& currentElemGroup = fst.ExclusivityGrouping;
			if (mappingGroup && currentElemGroup)
			{
				// If both have same ex. grouping
				if(mappingGroup.value_or(-1) == currentElemGroup.value_or(0))
				{
					// And if they aren't the same controller button (supports multiple maps per cb)
					if(mapping.Vk != fst.Vk)
					{
						iterBuffer.emplace_back(elem);
					}
				}
			}
		});
		return iterBuffer;
	}

	// Returns a vec of iterators
	inline
	auto GetMappingsMatchingVk(const int vk, MappingRange_c auto& mappingsList)
	-> std::vector<typename decltype(mappingsList)::iterator>
	{
		using MappingsList_t = decltype(mappingsList);
		using MappingsListIterator_t = typename MappingsList_t::iterator;
		using std::ranges::begin, std::ranges::end, std::ranges::for_each, std::ranges::cbegin, std::ranges::cend;

		// Create vec of pointers and add elements matching the VK
		std::vector<MappingsListIterator_t> buf;
		for_each(mappingsList, [vk, &buf](MappingsListIterator_t elem) { if (elem.first.Vk == vk) buf.emplace_back(elem); });
		return buf;
	}

	inline
	auto DoKeyUpForMatchingExclusivityGroupMappings(CBActionMap mapping, MappingRange_c auto& mappingsList)
	{
		// Handle exclusivity grouping members, they get sent key-up if they match this key-down's ex. group
		if (mapping.ExclusivityGrouping.has_value())
		{
			// Get a view to the elements matching the same exclusivity grouping.
			const auto rv = GetMatchingExclusivityGroupMappings(mapping, mappingsList);
			// Send key-up for the matching ex. group members.
			for (auto& et : rv)
			{
				if (et->second.IsDown() || et->second.IsRepeating())
				{
					// Send the up
					if (et->first.OnUp)
						et->first.OnUp.value()();
					// Update last sent time
					et->second.LastSentTime.Reset();
					// Update the last state
					et->second.SetUp();
				}
			}
		}
	}

	/**
	 * \brief DOES NOT CHECK THE STATE or the TIMER before sending, merely does the send & update logic.
	 *  NOTE that here, sending the key-ups for all the overtaken mappings in the same exclusivity grouping counts as "UPDATE"
	 */
	inline
	auto SendAndUpdateKeyDown(std::pair<CBActionMap, MappingStateManager>& mapping, MappingRange_c auto& mappingsList)
	{
		DoKeyUpForMatchingExclusivityGroupMappings(mapping.first, mappingsList);

		// test for handler
		if (mapping.first.OnDown)
			mapping.first.OnDown.value()();
		// Update the last state
		mapping.second.SetDown();
		// Update last sent time
		mapping.second.LastSentTime.Reset();
	}

	/**
	 * \brief DOES NOT CHECK THE STATE or the TIMER before sending, merely does the send & update logic.
	 */
	inline
	auto SendAndUpdateKeyRepeat(CBActionMap& mapping)
	{
		// note, it should not be possible for a repeat to trigger an exclusivity grouping overtaking action afaik
		// test for handler
		if (mapping.OnRepeat)
			mapping.OnRepeat.value()();

		// Update the last state
		mapping.LastAction.SetRepeat();

		// Update last sent time
		mapping.LastAction.LastSentTime.Reset();
	}

	/**
	 * \brief DOES NOT CHECK THE STATE or the TIMER before sending, merely does the send & update logic.
	 */
	inline
	auto SendAndUpdateKeyUp(CBActionMap& mapping)
	{
		// test for handler
		if (mapping.OnUp)
			mapping.OnUp.value()();

		mapping.LastAction.SetUp();

		// Update last sent time
		mapping.LastAction.LastSentTime.Reset();
	}

	//TODO function for cleaning up in-progress events.
	/**
	 * \brief This translator is responsible for managing the state regarding
	 * 1. exclusivity groupings,
	 * 2. the last action of the mapping,
	 * 3. the decision to do a repeat if the mapping has enabled repeat behavior,
	 * 4. the key-repeat and key-update timer loops
	 *
	 * To perform these tasks, it needs an internal working copy of every mapping in use.
	 */
	class CBActionTranslator
	{
	private:
		using Clock_t = std::chrono::high_resolution_clock;
		using TimePoint_t = DelayManagement::DelayManager<std::chrono::microseconds>;
		using LastAction_t = MappingStateManager;
		std::vector<std::pair<CBActionMap, LastAction_t>> m_mappings;
	public:
		/**
		 * \brief Throwing constructor, will not leave a partially constructed zombie object in the event of error.
		 * \param mappingsList STL container/range of CBActionMap controller button to action mappings.
		 * \throws std::invalid_argument exception
		 */
		CBActionTranslator(const std::ranges::range auto& mappingsList)
		{
			if (!AreExclusivityGroupsUnique(mappingsList))
				throw std::invalid_argument("Mapping list contained multiple exclusivity groupings for a single controller button.");
			for(const auto& e: mappingsList)
			{
				LastAction_t lastStateM{};
				if(e.CustomRepeatDelay)
					lastStateM.LastSentTime.Reset(e.CustomRepeatDelay.value());
				m_mappings.emplace_back(std::make_pair(e, lastStateM));
			}
		}
		
		auto ProcessState(const ControllerStateWrapper& state, MappingRange_c auto& mappingsList)
		-> std::vector<TranslationResult>
		{
			std::vector<TranslationResult> results;
			const auto matchingMappings = GetMappingsMatchingVk(state.VirtualKey, mappingsList);
			const auto updateMappings = GetMappingsForUpdate(mappingsList);
			const auto repeatMappings = GetMappingsForRepeat(mappingsList);


			// todo combine into one vec and return
			return results;
		}
		void ProcessState(const ControllerStateWrapper& state, const bool doUpdateLoop = true)
		{
			// Update timers/reset mappings.
			KeyUpdateLoop();
			// Send repeats
			KeyRepeatLoop();
			// Get a view to the elements matching the controller key vk
			const auto matchingMappings = GetMappingsMatchingVk(state.VirtualKey, m_mappings);
			// For each controller button matching mapping...
			for(auto currMapIt : matchingMappings)
			{
				// Ref to pointed-to mapping
				auto& currentElem = *currMapIt;
				// TODO The decision of whether to send down()/up() etc. needs to occur here
				if(state.KeyDown)
				{
					SendAndUpdateKeyDown(currentElem, matchingMappings);
				}
				else if(state.KeyUp)
				{
					SendAndUpdateKeyUp(currentElem, matchingMappings);
				}
				else if(state.KeyRepeat)
				{
					SendAndUpdateKeyRepeat(currentElem, matchingMappings);
				}
			}
		}
	private:
		/**
		 * \brief Retrieves a vector of iterators to elements requesting to be reset.
		 * If enough time has passed, the key requests to be rest for use again; provided it uses the key-repeat behavior--
		 * otherwise it requests to be reset immediately.
		 */
		auto GetMappingsForUpdate(MappingRange_c auto& mappingsList)
		{
			using std::ranges::begin, std::ranges::end;
			std::vector<TranslationResult> resetBuffer;
			for(auto elemIt = begin(mappingsList); elemIt != end(mappingsList); ++elemIt)
			{
				auto& elem = *elemIt;
				const bool DoUpdate = (elem.LastAction.IsUp() && elem.LastAction.LastSentTime.IsElapsed()) && elem.UsesRepeat;
				const bool DoImmediate = elem.LastAction.IsUp() && !elem.UsesRepeat;
				if (DoUpdate || DoImmediate)
				{
					resetBuffer.emplace_back(TranslationResult{false, false, false, true, &elem});
				}
			}
			return resetBuffer;
		}
		auto GetMappingsForRepeat(MappingRange_c auto& mappingsList)
		{
			using std::ranges::begin, std::ranges::end;
			std::vector<TranslationResult> repeatBuffer;
			for (auto elemIt = begin(mappingsList); elemIt != end(mappingsList); ++elemIt)
			{
				auto& w = *elemIt;
				const bool doesRepeat = w.UsesRepeat;
				const bool isDown = w.LastAction.IsDown();
				const bool isRepeating = w.LastAction.IsRepeating();
				const bool downOrRepeat = isDown || isRepeating;
				if (doesRepeat && downOrRepeat)
				{
					if (w.LastAction.LastSentTime.IsElapsed())
					{
						repeatBuffer.emplace_back(TranslationResult{ false, false, true, false, &w});
					}
				}
			}
			return repeatBuffer;
		}
	};
}

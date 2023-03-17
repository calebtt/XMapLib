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
		CBActionMap* ButtonMapping{};
	};

	template<typename T>
	concept MappingRange_c = requires (T & t)
	{
		{ t.begin() };
		{ t.end() };
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
	bool AreExclusivityGroupsUnique(const std::vector<CBActionMap>& mappingsList)
	{
		std::map<int, std::optional<int>> groupMap;
		for (const auto& e : mappingsList)
		{
			// If an exclusivity group is set, we must verify no duplicate ex groups are set to the same vk
			const auto& vk = e.Vk;
			const auto& currentMappingGroupOpt = e.ExclusivityGrouping;
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

	// Returns a vec of pointers, not iterators because we don't want to iterate at all.
	inline
	auto GetMappingsMatchingVk(const int vk, MappingRange_c auto& mappingsList)
	-> std::vector<typename decltype(mappingsList)::value_type*>
	{
		using MappingsList_t = decltype(mappingsList);
		using MappingsListIterator_t = typename MappingsList_t::iterator;
		using MappingsListPointer_t = typename MappingsList_t::value_type*;
		using std::ranges::begin, std::ranges::end, std::ranges::for_each, std::ranges::cbegin, std::ranges::cend;

		// Create vec of pointers and add elements matching the VK
		std::vector<MappingsListPointer_t> buf;
		for_each(mappingsList, [vk, &buf](auto& elem) { if (elem.first.Vk == vk) buf.emplace_back(&elem); });
		return buf;
	}
	/**
	 * \brief If enough time has passed, the key requests to be rest for use again; provided it uses the key-repeat behavior--
	 * otherwise it requests to be reset immediately.
	 */
	inline
	auto GetMappingsForUpdate(MappingRange_c auto& mappingsList) -> std::vector<TranslationResult>
	{
		using std::ranges::begin, std::ranges::end;
		std::vector<TranslationResult> resetBuffer;
		for (auto elemIt = begin(mappingsList); elemIt != end(mappingsList); ++elemIt)
		{
			auto& elem = *elemIt;
			const bool DoUpdate = (elem.LastAction.IsUp() && elem.LastAction.LastSentTime.IsElapsed()) && elem.UsesRepeat;
			const bool DoImmediate = elem.LastAction.IsUp() && !elem.UsesRepeat;
			if (DoUpdate || DoImmediate)
			{
				resetBuffer.emplace_back(TranslationResult{ false, false, false, true, &elem });
			}
		}
		return resetBuffer;
	}
	inline
	auto GetMappingsForRepeat(MappingRange_c auto& mappingsList) -> std::vector<TranslationResult>
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
					repeatBuffer.emplace_back(TranslationResult{ false, false, true, false, &w });
				}
			}
		}
		return repeatBuffer;
	}

	//TODO do something else with these send functions.
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
	 * To perform these tasks, it needs an internal working copy of every mapping in use,
	 * it encapsulates the mapping array.
	 */
	class CBActionTranslator
	{
	private:
		std::vector<CBActionMap> m_mappings;
		std::map<int, std::vector<CBActionMap*>> m_exGroupMap;
	public:
		/**
		 * \brief COPIES the mappings into the internal vector.
		 * Throwing constructor, will not leave a partially constructed zombie object in the event of error.
		 * \param mappingsList STL container/range of CBActionMap controller button to action mappings.
		 * \throws std::invalid_argument exception
		 */
		CBActionTranslator(const MappingRange_c auto& mappingsList)
		{
			if (!AreExclusivityGroupsUnique(mappingsList))
				throw std::invalid_argument("Mapping list contained multiple exclusivity groupings for a single controller button.");
			m_mappings.reserve(std::size(mappingsList));
			for(const CBActionMap& elem: mappingsList)
			{
				// Add to internal vector, possibly with custom repeat delay.
				m_mappings.emplace_back(elem);
				// If has an exclusivity grouping, add to map
				auto& tempBack = m_mappings.back();
				if (tempBack.CustomRepeatDelay)
					tempBack.LastAction.LastSentTime.Reset(tempBack.CustomRepeatDelay.value());
				if (tempBack.ExclusivityGrouping)
				{
					// Build map with pointers to elements of internal buffer.
					m_exGroupMap[*tempBack.ExclusivityGrouping].emplace_back(&tempBack);
				}
			}
		}
		// Move-ctor for mappings list.
		CBActionTranslator(MappingRange_c auto&& mappingsList)
		{
			if (!AreExclusivityGroupsUnique(mappingsList))
				throw std::invalid_argument("Mapping list contained multiple exclusivity groupings for a single controller button.");
			m_mappings.reserve(std::size(mappingsList));
			for (CBActionMap& elem : mappingsList)
			{
				// Add to internal vector, possibly with custom repeat delay.
				m_mappings.emplace_back(std::move(elem));
				// If has an exclusivity grouping, add to map
				auto& tempBack = m_mappings.back();
				if (tempBack.CustomRepeatDelay)
					tempBack.LastAction.LastSentTime.Reset(tempBack.CustomRepeatDelay.value());
				if (tempBack.ExclusivityGrouping)
				{
					// Build map with pointers to elements of internal buffer.
					m_exGroupMap[*tempBack.ExclusivityGrouping].emplace_back(&tempBack);
				}
			}
		}

		auto GetExGroupOvertaken(const CBActionMap& currentMapping) -> std::vector<TranslationResult>
		{
			std::vector<TranslationResult> results;
			// Iterate through each matching mapping and find ones with an exclusivity grouping, and then add the rest of the grouping to the results with key-up
			if(currentMapping.ExclusivityGrouping)
			{
				auto& tempVec = m_exGroupMap[*currentMapping.ExclusivityGrouping];
				for (CBActionMap* p : tempVec)
				{
					if (p != &currentMapping)
					{
						if (p->LastAction.IsDown() || p->LastAction.IsRepeating())
							results.emplace_back(TranslationResult{ false, true, false, false, p });
					}
				}
			}
			
			return results;
		}

		auto ProcessState(const ControllerStateWrapper& state)
		-> std::vector<TranslationResult>
		{
			using std::ranges::find_if;
			std::vector<TranslationResult> results;
			// Adds maps being reset
			results.append_range(GetMappingsForUpdate(m_mappings));
			// Adds maps being key-repeat'd
			results.append_range(GetMappingsForRepeat(m_mappings));
			// Adds maps being overtaken and sent a key-up
			auto matchingMappings = GetMappingsMatchingVk(state.VirtualKey, m_mappings);
			for(const auto &elem : matchingMappings)
			{
				results.append_range(GetExGroupOvertaken(elem));
			}
			// Adds maps doing key-down for matching the controller button
			results.append_range(matchingMappings);
			return results;
		}
	};
}

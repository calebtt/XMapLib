#pragma once
#include "LibIncludes.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include <iostream>
#include <chrono>
#include <optional>
#include <vector>
#include <iterator>
#include <ostream>
#include <tuple>
#include <ranges>
#include <type_traits>

namespace sds
{
	struct TranslationResult
	{
		bool DoDown{ false };
		bool DoUp{ false };
		bool DoRepeat{ false };
		bool DoReset{ false };
		CBActionMap* ButtonMapping{};
		// Debugging purposes
		friend auto operator<<(std::ostream& os, const TranslationResult& obj) -> std::ostream&
		{
			return os
				<< "DoDown: " << obj.DoDown
				<< " DoUp: " << obj.DoUp
				<< " DoRepeat: " << obj.DoRepeat
				<< " DoReset: " << obj.DoReset
				<< " ButtonMapping: " << obj.ButtonMapping;
		}
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
	auto GetMappingsMatchingVk(const int vk, std::vector<CBActionMap>& mappingsList)
	{
		// TODO a function that makes translation results from the ret val
		using MappingsListPointer_t = CBActionMap*;
		using std::ranges::for_each;

		// Create vec of pointers and add elements matching the VK
		std::vector<MappingsListPointer_t> buf;
		for_each(mappingsList, [vk, &buf](auto& elem) { if (elem.Vk == vk) buf.emplace_back(&elem); });
		return buf;
	}

	/**
	 * \brief If enough time has passed, the key requests to be reset for use again; provided it uses the key-repeat behavior--
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
	 * \remarks Construct a new instance to encapsulate a new or altered set of mappings.
	 */
	class KeyboardActionTranslator
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
		KeyboardActionTranslator(const MappingRange_c auto& mappingsList)
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
		KeyboardActionTranslator(MappingRange_c auto&& mappingsList)
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
	public:
		// Gets state update actions, typical usage of the type.
		auto operator()(const ControllerStateWrapper& state) -> std::vector<TranslationResult>
		{
			return GetStateUpdateActions(state);
		}

		/**
		 * \brief The returned translationresult updates are in-order,
		 * 1. Mappings being reset
		 * 2. Mappings being key-repeat'd
		 * 3. Mappings being overtaken by the next key-down
		 * 4. Mappings being sent a (initial) key-down
		 * 5. Mappings being sent a key-up
		 * 6. todo
		 */
		auto GetStateUpdateActions(const ControllerStateWrapper& state)
		-> std::vector<TranslationResult>
		{
			using std::ranges::find_if;
			std::vector<TranslationResult> results;
			// Adds maps with timer being reset
			results.append_range(GetMappingsForUpdate(m_mappings));
			// Adds maps being key-repeat'd
			results.append_range(GetMappingsForRepeat(m_mappings));
			// Adds maps being overtaken and sent a key-up
			auto matchingMappings = GetMappingsMatchingVk(state.VirtualKey, m_mappings);
			for(const auto elem : matchingMappings)
			{
				results.append_range(GetExGroupOvertaken(*elem));
			}
			for(const auto elem : matchingMappings)
			{
				// TODO make all of this simpler, storing which mappings have been sent may be the best idea.
				// or duplicating the mapping into another array
				const auto alreadySent = find_if(results, [&](const auto& curResult) { return curResult.ButtonMapping == elem; });
				// TODO this doesn't distinguish between up/down/etc. either
				results.emplace_back(TranslationResult{ true, false, false, false, elem });
			}

			return results;
		}

		/**
		 * \brief Intended to provide an array of key-up actions necessary to return the mappings back to an initial state.
		 */
		auto GetCleanupActions() -> std::vector<TranslationResult>
		{
			std::vector<TranslationResult> results;
			for(auto& elem: m_mappings)
			{
				if(elem.LastAction.IsDown() || elem.LastAction.IsRepeating())
				{
					results.emplace_back(TranslationResult{ .DoDown = false, .DoUp = true, .DoRepeat = false, .DoReset = false, .ButtonMapping = &elem });
				}
			}
			return results;
		}
	private:
		auto GetExGroupOvertaken(const CBActionMap& currentMapping) -> std::vector<TranslationResult>
		{
			// TODO this might add duplicates of the same action, will need tested.
			using std::ranges::find_if;
			std::vector<TranslationResult> results;
			// Iterate through each matching mapping and find ones with an exclusivity grouping, and then add the rest of the grouping to the results with key-up
			if (currentMapping.ExclusivityGrouping)
			{
				auto& exGroupVecForCurrent = m_exGroupMap[*currentMapping.ExclusivityGrouping];
				for (CBActionMap* p : exGroupVecForCurrent)
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
	};
}

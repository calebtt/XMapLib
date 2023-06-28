#pragma once
#include "LibIncludes.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardTranslationResult.h"

#include <chrono>
#include <optional>
#include <vector>
#include <iterator>
#include <ostream>
#include <tuple>
#include <ranges>
#include <set>
#include <type_traits>
#include <span>
#include <unordered_set>
#include <algorithm>

namespace sds
{
	/**
	 * \brief	Initializes the MappingStateManager timers with custom time delays from the mapping.
	 * \param mappingElem	The controller button to action mapping, possibly with the optional custom delay values.
	 */
	inline
	void InitCustomTimers(CBActionMap& mappingElem) noexcept
	{
		if (mappingElem.DelayForRepeats)
			mappingElem.LastAction.LastSentTime.Reset(mappingElem.DelayForRepeats.value());
		if (mappingElem.DelayBeforeFirstRepeat)
			mappingElem.LastAction.DelayBeforeFirstRepeat.Reset(mappingElem.DelayBeforeFirstRepeat.value());
	}
	[[nodiscard]]
	inline
	auto GetResetTranslationResult(CBActionMap& currentMapping) noexcept -> TranslationResult
	{
		return TranslationResult
		{
			.OperationToPerform = [&currentMapping]() {
				if (currentMapping.OnReset)
					currentMapping.OnReset();
			},
			.AdvanceStateFn = [&currentMapping]() {
				currentMapping.LastAction.SetInitial();
				currentMapping.LastAction.LastSentTime.Reset();
			}
		};
	}
	[[nodiscard]]
	inline
	auto GetRepeatTranslationResult(CBActionMap& currentMapping) noexcept -> TranslationResult
	{
		return TranslationResult
		{
			.OperationToPerform = [&currentMapping]() {
				if (currentMapping.OnRepeat)
					currentMapping.OnRepeat();
				currentMapping.LastAction.LastSentTime.Reset();
			},
			.AdvanceStateFn = [&currentMapping]() {
				currentMapping.LastAction.SetRepeat();
			}
		};
	}
	[[nodiscard]]
	inline
	auto GetOvertakenTranslationResult(CBActionMap& overtakenMapping) noexcept -> TranslationResult
	{
		return TranslationResult
		{
			.OperationToPerform = [&overtakenMapping]()
			{
				if (overtakenMapping.OnUp)
					overtakenMapping.OnUp();
			},
			.AdvanceStateFn = [&overtakenMapping]()
			{
				overtakenMapping.LastAction.SetUp();
			}
		};
	}
	[[nodiscard]]
	inline
	auto GetKeyUpTranslationResult(CBActionMap& currentMapping) noexcept -> TranslationResult
	{
		return TranslationResult
		{
			.OperationToPerform = [&currentMapping]()
			{
				if (currentMapping.OnUp)
					currentMapping.OnUp();
			},
			.AdvanceStateFn = [&currentMapping]()
			{
				currentMapping.LastAction.SetUp();
			}
		};
	}
	[[nodiscard]]
	inline
	auto GetInitialKeyDownTranslationResult(CBActionMap& currentMapping) noexcept -> TranslationResult
	{
		return TranslationResult
		{
			.OperationToPerform = [&currentMapping]()
			{
				if (currentMapping.OnDown)
					currentMapping.OnDown();
				// Reset timer after activation, to wait for elapsed before another next state translation is returned.
				currentMapping.LastAction.LastSentTime.Reset();
				currentMapping.LastAction.DelayBeforeFirstRepeat.Reset();
			},
			.AdvanceStateFn = [&currentMapping]()
			{
				currentMapping.LastAction.SetDown();
			}
		};
	}
	/**
	 * \brief	Checks a list of mappings for having multiple exclusivity groupings mapped to a single controller button.
	 * \param	mappingsList Vector of controller button to action mappings.
	 * \return	true if good mapping list, false if there is a problem.
	 */
	[[nodiscard]]
	inline
	bool AreExclusivityGroupsUnique(const std::vector<CBActionMap>& mappingsList) noexcept
	{
		std::map<int, std::optional<int>> groupMap;
		for (const auto& e : mappingsList)
		{
			// If an exclusivity group is set, we must verify no duplicate ex groups are set to the same vk
			const auto& vk = e.ButtonVirtualKeycode;
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


	// TODO might not need this
	/**
	 * \brief By returning the index, we can avoid issues with iterator/pointer invalidation.
	 * Uses unsigned int because size_t is just a waste of space here.
	 * \param vk Controller Button Virtual Keycode
	 * \param mappingsList List of controller button to action mappings.
	 * \return Vector of indices at which mappings which map to the controller button VK can be located.
	 */
	[[nodiscard]]
	constexpr
	auto GetVkMatchIndices(const int vk, const std::vector<sds::CBActionMap>& mappingsList) noexcept -> std::vector<std::uint32_t>
	{
		std::vector<std::uint32_t> buf;
		for (std::uint32_t i{}; i < mappingsList.size(); ++i)
		{
			const auto& elem = mappingsList[i];
			if (elem.ButtonVirtualKeycode == vk)
				buf.emplace_back(i);
		}
		return buf;
	}
	/**
	 * \brief If enough time has passed, the key requests to be reset for use again; provided it uses the key-repeat behavior--
	 * otherwise it requests to be reset immediately.
	 */
	[[nodiscard]]
	inline
	auto GetUpdateIndices(const std::vector<CBActionMap>& mappingsList) noexcept -> std::vector<std::uint32_t>
	{
		using std::ranges::for_each;
		std::vector<std::uint32_t> resetBuffer;
		for (std::uint32_t i{}; i < mappingsList.size(); ++i)
		{
			const auto& elem = mappingsList[i];
			const bool DoUpdate = (elem.LastAction.IsUp() && elem.LastAction.LastSentTime.IsElapsed()) && elem.UsesInfiniteRepeat;
			const bool DoImmediate = elem.LastAction.IsUp() && !elem.UsesInfiniteRepeat;
			if (DoUpdate || DoImmediate)
			{
				resetBuffer.emplace_back(i);
			}
		}
		return resetBuffer;
	}
	/**
	 * \brief Returns vec of index to mappings that require a key-repeat (timer has elapsed, key is doing key-repeat).
	 */
	[[nodiscard]]
	inline
	auto GetRepeatIndices(const std::vector<CBActionMap>& mappingsList) noexcept -> std::vector<std::uint32_t>
	{
		using std::ranges::for_each;
		std::vector<std::uint32_t> buf;
		for (std::uint32_t i{}; i < mappingsList.size(); ++i)
		{
			const auto& elem = mappingsList[i];
			const bool doesInfiniteRepeat = elem.UsesInfiniteRepeat;
			const bool doesSingleRepeat = elem.SendsFirstRepeatOnly;
			const bool isDown = elem.LastAction.IsDown();
			const bool isRepeating = elem.LastAction.IsRepeating();
			const bool isElapsed = elem.LastAction.LastSentTime.IsElapsed();
			const bool isInitialRepeatDelayElapsed = elem.LastAction.DelayBeforeFirstRepeat.IsElapsed();

			const bool doSingleRepeat = !doesInfiniteRepeat && doesSingleRepeat && isDown && isInitialRepeatDelayElapsed;
			const bool doInitialRepeat = doesInfiniteRepeat && isDown && isInitialRepeatDelayElapsed;
			const bool doRepeatRepeat = doesInfiniteRepeat && isRepeating && isElapsed;
			if (doInitialRepeat || doRepeatRepeat || doSingleRepeat)
				buf.emplace_back(i);
		}
		return buf;
	}
	/**
	 * \brief	Used to find the elements of "toAdd" that aren't in "existing".
	 * \param existing	Existing range of elements
	 * \param toAdd		Range of elements tested for adding to existing.
	 * \return	Returns the elements of toAdd that aren't already in existing.
	 */
	inline
	auto GetUniqueMatches(const std::vector<std::uint32_t>& existing, const std::vector<std::uint32_t>& toAdd) noexcept -> std::vector<std::uint32_t>
	{
		using std::size_t, std::ranges::sort, std::ranges::binary_search;
		using std::vector, std::uint32_t;
		vector existingSet(existing);
		sort(existingSet);
		vector<uint32_t> uniqueMatchResult;
		for (size_t i{}; i < toAdd.size(); ++i)
		{
			const auto e = toAdd[i];
			if (!binary_search(existingSet, e))
				uniqueMatchResult.emplace_back(e);
		}
		return uniqueMatchResult;
	}

	// Hopefully behavior not too specific so as to become unusable for someone trying to add a customization.
	// Customization class for exclusivity group behavior. TODO refactor to work
	class OvertakingBehavior
	{
		std::map<int, std::vector<std::uint32_t>> m_exGroupMap;
		bool m_firstCall{ true };
	public:
		// This function accepts the index of keys being key-down'd and will produce the appropriate vector of TranslationResult
		// for the Exclusivity Group handling, key-up in this implementation.
		auto GetOvertakenTranslationResultsFor(std::span<CBActionMap> mappingsList, const std::uint32_t currentIndex) -> std::vector<TranslationResult>
		{
			using std::ranges::find_if;
			if(m_firstCall)
			{
				// Build the map of ex. group integer to vector of pointers to the mapping(s).
				for(std::size_t i{}; i < mappingsList.size(); ++i)
				{
					auto& elem = mappingsList[i];
					// If has an exclusivity grouping, add to map
					if (elem.ExclusivityGrouping)
						m_exGroupMap[*elem.ExclusivityGrouping].emplace_back((std::uint32_t)i);
				}
				m_firstCall = false;
			}
			// Iterate through each matching mapping and find ones with an exclusivity grouping, and then add the rest of the grouping to the results with key-up
			const auto& currentMap = mappingsList[currentIndex];
			if (currentMap.ExclusivityGrouping)
			{
				std::vector<TranslationResult> results;
				const auto& exGroupVecForCurrent = m_exGroupMap[*currentMap.ExclusivityGrouping];
				for (const auto elemIndex: exGroupVecForCurrent)
				{
					if (elemIndex != currentIndex)
					{
						auto& testElem = mappingsList[elemIndex];
						if (testElem.LastAction.IsDown() || testElem.LastAction.IsRepeating())
						{
							results.emplace_back(GetOvertakenTranslationResult(testElem));
							// TODO this is returning a bad overtaken result for ltrigger down then pressing rtrigger, don't think it's the poller!
						}
					}
				}
				return results;
			}
			return {};
		}
	};
}

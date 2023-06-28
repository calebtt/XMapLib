#pragma once
#include "KeyboardLibIncludes.h"
#include "ControllerButtonToActionMap.h"

#include <optional>
#include <vector>
#include <ostream>
#include <type_traits>
#include <span>
#include <algorithm>

namespace sds
{
	/**
	 * \brief	TranslationResult holds info from a translated state change, typically the operation to perform (if any) and
	 *	a function to call to advance the state to the next state to continue to receive proper translation results.
	 */
	struct TranslationResult
	{
		// Operation being requested to be performed, callable
		detail::Fn_t OperationToPerform;
		// Function to advance the button mapping to the next state (after operation has been performed)
		detail::Fn_t AdvanceStateFn;
		// Call operator, calls op fn then advances the state
		void operator()() const
		{
			OperationToPerform();
			AdvanceStateFn();
		}
	};
	static_assert(std::copyable<TranslationResult>);
	static_assert(std::movable<TranslationResult>);

	/**
	 * \brief	TranslationPack is a pack of ranges containing individual TranslationResult structs for processing
	 *	state changes.
	 */
	struct TranslationPack
	{
		void operator()() const
		{
			// Note that there will be a function called if there is a state change,
			// it just may not have any custom behavior attached to it.
			for (const auto& elem : UpdateRequests)
				elem();
			for (const auto& elem : OvertakenRequests)
				elem();
			for (const auto& elem : RepeatRequests)
				elem();
			for (const auto& elem : NextStateRequests)
				elem();
		}
		// TODO might wrap the vectors in a struct with a call operator to have individual call operators for range of TranslationResult.
		detail::SmallVector_t<TranslationResult> UpdateRequests{};
		detail::SmallVector_t<TranslationResult> RepeatRequests{};
		detail::SmallVector_t<TranslationResult> OvertakenRequests{};
		detail::SmallVector_t<TranslationResult> NextStateRequests{};
	};
	static_assert(std::copyable<TranslationPack>);
	static_assert(std::movable<TranslationPack>);

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
						m_exGroupMap[*elem.ExclusivityGrouping].emplace_back(static_cast<std::uint32_t>(i));
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

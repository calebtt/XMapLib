#pragma once
#include "LibIncludes.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPollerController.h"
#include "ButtonStateMgr.h"
#include "KeyboardTranslationResult.h"

#include <iostream>
#include <chrono>
#include <optional>
#include <vector>
#include <iterator>
#include <ostream>
#include <tuple>
#include <ranges>
#include <set>
#include <type_traits>

namespace sds
{
	template<typename T>
	concept MappingRange_c = requires (T & t)
	{
		{ std::same_as<typename T::value_type, CBActionMap> };
		{ std::ranges::forward_range<T> };
	};

	[[nodiscard]] inline bool AreExclusivityGroupsUnique(const std::vector<CBActionMap>& mappingsList);
	[[nodiscard]] inline auto GetUpdateIndices(const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>;
	[[nodiscard]] inline auto GetRepeatIndices(const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>;
	[[nodiscard]] constexpr auto GetVkMatchIndices(const int vk, const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>;
	[[nodiscard]] inline auto GetUniqueMatches(const std::vector<std::uint32_t> existing, const std::vector<std::uint32_t> toAdd) -> std::vector<std::uint32_t>;
	inline void InitCustomTimers(CBActionMap& mappingElem); // Init some custom timers, if present

	[[nodiscard]] inline auto GetUpdateTranslationResult(CBActionMap& currentMapping) -> TranslationResult;
	[[nodiscard]] inline auto GetRepeatTranslationResult(CBActionMap& currentMapping) -> TranslationResult;
	[[nodiscard]] inline auto GetOvertakenTranslationResult(CBActionMap& overtakenMapping) -> TranslationResult;
	[[nodiscard]] inline auto GetKeyUpTranslationResult(CBActionMap& currentMapping) -> TranslationResult;
	[[nodiscard]] inline auto GetInitialKeyDownTranslationResult(CBActionMap& currentMapping) -> TranslationResult;

	//// Hopefully behavior not too specific so as to become unusable
	//// for someone trying to add a customization. Customization class for exclusivity group
	//// behavior.
	//class OvertakingBehavior
	//{
	//	std::span<CBActionMap> m_mappings;
	//	std::map<int, std::vector<CBActionMap*>> m_exGroupMap;
	//public:
	//	// TODO complete this
	// // TODO might extract exclusivity grouping code into an object used by the translator, could be a template param
	//	OvertakingBehavior(std::span<CBActionMap> mappingsList) : m_mappings(mappingsList)
	//	{
	//		for (CBActionMap& elem : mappingsList)
	//		{
	//			// If has an exclusivity grouping, add to map
	//			if (elem.ExclusivityGrouping)
	//				m_exGroupMap[*elem.ExclusivityGrouping].emplace_back(&elem);
	//		}
	//	}
	//	auto GetOvertakenTranslationResultsFor(const std::uint32_t currentIndex) -> std::vector<TranslationResult>
	//	{
	//		return {};
	//	}
	//};

	/**
	 * \brief This translator is responsible for managing the state of
	 * 1. exclusivity groupings,
	 * 2. the last action of the mapping,
	 * 3. the decision to do a repeat if the mapping has enabled repeat behavior,
	 * 4. the key-repeat and key-update timer loops
	 * 5. *Note that this translator doesn't make the updates to the mapping, only notifies when they should occur.
	 *
	 * To perform these tasks, it needs an internal working copy of every mapping in use, it encapsulates the mapping array.
	 * \remarks Construct a new instance to encapsulate a new or altered set of mappings.
	 * Key mappings only traverse states in these paths, note they begin at "initial":
	 * 1. initial -> down -> repeat -> up
	 * 2. initial -> down -> up
	 */
	class KeyboardActionTranslator
	{
		static constexpr std::string_view ExclusivityGroupError{ "Mapping list contained multiple exclusivity groupings for a single controller button." };
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
			// TODO add check for sends single repeat and uses repeat in an invalid config.
			if (!AreExclusivityGroupsUnique(mappingsList))
				throw std::invalid_argument(ExclusivityGroupError.data());
			InitMappingDetails(mappingsList);
		}
		// Move-ctor for mappings list.
		KeyboardActionTranslator(std::vector<CBActionMap>&& mappingsList)
		{
			if (!AreExclusivityGroupsUnique(mappingsList))
				throw std::invalid_argument(ExclusivityGroupError.data());
			InitMappingDetails(std::move(mappingsList));
		}
	public:
		// Gets state update actions, typical usage of the type.
		auto operator()(const ControllerStateWrapper& state) -> TranslationPack
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
		auto GetStateUpdateActions(const ControllerStateWrapper& buttonInfo)
		-> TranslationPack
		{
			using std::ranges::find_if, std::erase_if, std::ranges::begin, std::ranges::end, std::ranges::cbegin, std::ranges::cend;
			using std::ranges::find, std::ranges::transform;

			// Get update, repeat, direct match, etc. indices lists
			const auto matchingIndices = GetVkMatchIndices(buttonInfo.VirtualKey, m_mappings); // Gets all indices matching VK
			
			const auto updateIndices = GetUpdateIndices(m_mappings); // Gets update/reset indices
			auto repeatIndices = GetRepeatIndices(m_mappings); // Gets key-repeat indices
			// Remove from the repeat indices all of the ones matching the VK when buttonInfo is key-up (progression)
			if (buttonInfo.KeyUp)
			{
				erase_if(repeatIndices, [&](const auto e)
				{
					return m_mappings[e].Vk == buttonInfo.VirtualKey;
				});
			}

			const auto uniqueMatches = GetUniqueMatches(repeatIndices, matchingIndices); // Filters out the key-repeat indices

			TranslationPack tPack;
			// Get exclusivity group overtaken, but not on a key-up
			if (!buttonInfo.KeyUp)
			{
				for (const auto cur : uniqueMatches)
				{
					tPack.OvertakenRequests.append_range(GetOvertakenTranslationResultsFor(cur));
				}
			}
			// Adds maps with timer being reset (updated)
			transform(updateIndices, std::back_inserter(tPack.UpdateRequests), [&](const auto n)
			{
				return GetUpdateTranslationResult(MappingAt(n));
			});
			// Adds maps being key-repeat'd
			transform(repeatIndices, std::back_inserter(tPack.RepeatRequests), [&](const auto n)
			{
				return GetRepeatTranslationResult(MappingAt(n));
			});
			// Add direct translations for each unique
			for(const auto matchInd : uniqueMatches)
			{
				GetDirectTranslations(buttonInfo, tPack.NextStateRequests, matchInd);
			}

			return tPack;
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
					results.emplace_back(GetKeyUpTranslationResult(elem));
				}
			}
			return results;
		}
	private:
		constexpr
		auto MappingAt(const std::uint32_t index) noexcept -> CBActionMap&
		{
			return m_mappings.at(index);
		}
		auto GetOvertakenTranslationResultsFor(const std::uint32_t currentIndex) -> std::vector<TranslationResult>
		{
			using std::ranges::find_if;
			// Iterate through each matching mapping and find ones with an exclusivity grouping, and then add the rest of the grouping to the results with key-up
			const auto& currentMap = m_mappings[currentIndex];
			if (currentMap.ExclusivityGrouping)
			{
				std::vector<TranslationResult> results;
				const auto& exGroupVecForCurrent = m_exGroupMap[*currentMap.ExclusivityGrouping];
				for (CBActionMap* p : exGroupVecForCurrent)
				{
					if (p != &currentMap)
					{
						if (p->LastAction.IsDown() || p->LastAction.IsRepeating())
						{
							results.emplace_back(GetOvertakenTranslationResult(*p));
						}
					}
				}
				return results;
			}
			return {};
		}
		void GetDirectTranslations(
			const ControllerStateWrapper& buttonInfo, 
			std::vector<TranslationResult>& results, 
			const std::uint32_t matchInd)
		{
			auto& currentMapping = m_mappings[matchInd];
			const bool isMapInit = currentMapping.LastAction.IsInitialState();
			const bool isMapDown = currentMapping.LastAction.IsDown();
			const bool isMapRepeat = currentMapping.LastAction.IsRepeating();
			const bool isButtonDown = buttonInfo.KeyDown;
			const bool isButtonUp = buttonInfo.KeyUp;

			// Initial key-down case
			if (isButtonDown && isMapInit)
			{
				results.emplace_back(GetInitialKeyDownTranslationResult(currentMapping));
			}
			// Key-up case
			if (isButtonUp && (isMapDown || isMapRepeat))
			{
				// Key-up doesn't require timer elapsed.
				results.emplace_back(GetKeyUpTranslationResult(currentMapping));
			}
		}
		void InitMappingDetails(const MappingRange_c auto& mappingsList)
		{
			m_mappings.reserve(std::size(mappingsList));
			for (const CBActionMap& elem : mappingsList)
			{
				// Add to internal vector, possibly with custom repeat delay.
				m_mappings.emplace_back(elem);
				auto& tempBack = m_mappings.back();
				InitCustomTimers(tempBack);
				// If has an exclusivity grouping, add to map
				if (tempBack.ExclusivityGrouping)
					m_exGroupMap[*tempBack.ExclusivityGrouping].emplace_back(&tempBack);
			}
		}
		void InitMappingDetails(std::vector<CBActionMap>&& mappingsList)
		{
			m_mappings = std::move(mappingsList);
			for(auto& elem: m_mappings)
			{
				InitCustomTimers(elem);
				// If has an exclusivity grouping, add to map
				if (elem.ExclusivityGrouping)
					m_exGroupMap[*elem.ExclusivityGrouping].emplace_back(&elem);
			}
		}
	};

	/**
	 * \brief	Checks a list of mappings for having multiple exclusivity groupings mapped to a single controller button.
	 * \param	mappingsList Vector of controller button to action mappings.
	 * \return	true if good mapping list, false if there is a problem.
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

	/**
	 * \brief If enough time has passed, the key requests to be reset for use again; provided it uses the key-repeat behavior--
	 * otherwise it requests to be reset immediately.
	 */
	inline
	auto GetUpdateIndices(const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>
	{
		using std::ranges::for_each;
		std::uint32_t currentLocation{};
		std::vector<std::uint32_t> resetBuffer;
		for_each(mappingsList, [&resetBuffer, &currentLocation](const auto& elem)
			{
				const bool DoUpdate = (elem.LastAction.IsUp() && elem.LastAction.LastSentTime.IsElapsed()) && elem.UsesInfiniteRepeat;
				const bool DoImmediate = elem.LastAction.IsUp() && !elem.UsesInfiniteRepeat;
				if (DoUpdate || DoImmediate)
				{
					resetBuffer.emplace_back(currentLocation);
				}
				++currentLocation;
			});
		return resetBuffer;
	}

	/**
	 * \brief Returns vec of index to mappings that require a key-repeat (timer has elapsed, key is doing key-repeat).
	 */
	inline
	auto GetRepeatIndices(const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>
	{
		using std::ranges::for_each;

		std::vector<std::uint32_t> buf;
		for(std::uint32_t i{}; i < mappingsList.size(); ++i)
		{
			const auto& elem = mappingsList[i];
			const bool doesInfiniteRepeat = elem.UsesInfiniteRepeat;
			const bool doesSingleRepeat = elem.SendsFirstRepeatOnly;
			const bool isDown = elem.LastAction.IsDown();
			const bool isRepeating = elem.LastAction.IsRepeating();
			const bool isElapsed = elem.LastAction.LastSentTime.IsElapsed();
			const bool isInitialRepeatDelayElapsed = elem.LastAction.DelayBeforeFirstRepeat.IsElapsed();

			const bool doSingleRepeat = !doesInfiniteRepeat  && doesSingleRepeat && isDown && isInitialRepeatDelayElapsed;
			const bool doInitialRepeat = doesInfiniteRepeat && isDown && isInitialRepeatDelayElapsed;
			const bool doRepeatRepeat = doesInfiniteRepeat && isRepeating && isElapsed;
			if(doInitialRepeat || doRepeatRepeat || doSingleRepeat)
				buf.emplace_back(i);
		}
		return buf;
	}

	inline
	auto GetUniqueMatches(const std::vector<std::uint32_t> existing, const std::vector<std::uint32_t> toAdd) -> std::vector<std::uint32_t>
	{
		using std::ranges::find, std::ranges::end, std::ranges::begin, std::ranges::transform, std::size_t;
		// TODO use some of the algo header funcs, possibly set_intersection or merge or similar
		std::vector<std::uint32_t> uniqueMatchResult;
		// Don't add existing indices to a set or anything, cpu arch will speed up iterating an array 1-100x vs iterating a r-b tree.
		for (size_t i{}; i < toAdd.size(); ++i)
		{
			const auto e = toAdd[i];
			if (find(existing, e) == end(existing))
				uniqueMatchResult.emplace_back(e);
		}
		return uniqueMatchResult;
	}

	/**
	 * \brief By returning the index, we can avoid issues with iterator/pointer invalidation.
	 * Uses unsigned int because size_t is just a waste of space here.
	 * \param vk Controller Button Virtual Keycode
	 * \param mappingsList List of controller button to action mappings.
	 * \return Vector of indices at which mappings which map to the controller button VK can be located.
	 */
	constexpr
	auto GetVkMatchIndices(const int vk, const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>
	{
		using std::ranges::for_each;

		std::vector<std::uint32_t> buf;
		for (std::uint32_t i{}; i < mappingsList.size(); ++i)
		{
			const auto& elem = mappingsList[i];
			if (elem.Vk == vk)
				buf.emplace_back(i);
		}
		return buf;
	}

	/**
	 * \brief	Initializes the MappingStateManager timers with custom time delays from the mapping.
	 * \param mappingElem	The controller button to action mapping, possibly with the optional custom delay values.
	 */
	inline
	void InitCustomTimers(CBActionMap& mappingElem)
	{
		if (mappingElem.DelayForRepeats)
			mappingElem.LastAction.LastSentTime.Reset(mappingElem.DelayForRepeats.value());
		if (mappingElem.DelayBeforeFirstRepeat)
			mappingElem.LastAction.DelayBeforeFirstRepeat.Reset(mappingElem.DelayBeforeFirstRepeat.value());
	}

	[[nodiscard]]
	inline
	auto GetUpdateTranslationResult(CBActionMap& currentMapping) -> TranslationResult
	{
		return TranslationResult
		{
			.DoState = ButtonStateMgrInitial(),
			.OperationToPerform = [&currentMapping](){
				if (currentMapping.OnReset)
					currentMapping.OnReset();
			},
			.AdvanceStateFn = [&currentMapping](){
				currentMapping.LastAction.SetInitial();
				currentMapping.LastAction.LastSentTime.Reset();
			}
		};
	}

	[[nodiscard]]
	inline
	auto GetRepeatTranslationResult(CBActionMap& currentMapping) -> TranslationResult
	{
		return TranslationResult
		{
			.DoState = ButtonStateMgrRepeat(),
			.OperationToPerform = [&currentMapping](){
				if (currentMapping.OnRepeat)
					currentMapping.OnRepeat();
				currentMapping.LastAction.LastSentTime.Reset();
			},
			.AdvanceStateFn = [&currentMapping](){
				currentMapping.LastAction.SetRepeat();
			}
		};
	}

	[[nodiscard]]
	inline
	auto GetOvertakenTranslationResult(CBActionMap& overtakenMapping) -> TranslationResult
	{
		return TranslationResult
		{
			.DoState = ButtonStateMgrUp(),
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
	auto GetKeyUpTranslationResult(CBActionMap& currentMapping) -> TranslationResult
	{
		return TranslationResult
		{
			.DoState = ButtonStateMgrUp(),
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
	auto GetInitialKeyDownTranslationResult(CBActionMap& currentMapping) -> TranslationResult
	{
		return TranslationResult
		{
			.DoState = ButtonStateMgrDown(),
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
}

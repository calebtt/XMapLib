#pragma once
#include "LibIncludes.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include "PriorityMgr.h"
#include "ButtonStateMgr.h"

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
		{ t.begin() };
		{ t.end() };
		{ std::same_as<typename T::value_type, CBActionMap> };
	};
	struct TranslationResult
	{
		// Action to perform
		ButtonStateMgr DoState;
		// Mapping associated with translation result
		CBActionMap* ButtonMapping{};
		// Priority type for action
		PriorityMgr Priority; //TODO might remove this, don't think I need it.

		//TODO extract info relevant to operation from ptr to mapping, send only that instead
		//TODO maybe...
		//// Virtual keycode of controller button being activated
		//int ControllerVk{};
		//// Operation being requested to be performed
		//detail::OptFn_t OpRequested;
		

		// Debugging purposes
		friend auto operator<<(std::ostream& os, const TranslationResult& obj) -> std::ostream&
		{
			return os
				<< "DoDown: " << obj.DoState.IsDown()
				<< " DoRepeat: " << obj.DoState.IsRepeating()
				<< " DoUp: " << obj.DoState.IsUp()
				<< " DoReset: " << obj.DoState.IsInitialState()
				<< " ButtonMapping: " << obj.ButtonMapping;
		}
	};

	struct TranslationPack
	{
		std::vector<TranslationResult> UpdateRequests;
		std::vector<TranslationResult> RepeatRequests;
		std::vector<TranslationResult> OvertakenRequests;
		std::vector<TranslationResult> NextStateRequests;
	};

	[[nodiscard]] inline bool AreExclusivityGroupsUnique(const std::vector<CBActionMap>& mappingsList);
	[[nodiscard]] inline auto GetMappingsMatchingVk(const int vk, std::vector<CBActionMap>& mappingsList) -> std::vector<CBActionMap*>;
	[[nodiscard]] inline auto GetUpdateIndices(const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>;
	[[nodiscard]] inline auto GetRepeatIndices(const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>;
	[[nodiscard]] constexpr auto GetVkMatchIndices(const int vk, const std::vector<CBActionMap>& mappingsList) -> std::vector<std::uint32_t>;
	[[nodiscard]] inline auto GetUniqueMatches(const std::vector<std::uint32_t> existing, const std::vector<std::uint32_t> toAdd) -> std::vector<std::uint32_t>;

	/**
	 * \brief This translator is responsible for providing info regarding the state of
	 * 1. exclusivity groupings,
	 * 2. the last action of the mapping,
	 * 3. the decision to do a repeat if the mapping has enabled repeat behavior,
	 * 4. the key-repeat and key-update timer loops
	 * 5. *Note that this translator doesn't make the updates to the mapping, only notifies when they should occur.
	 *
	 * To perform these tasks, it needs an internal working copy of every mapping in use,
	 * it encapsulates the mapping array.
	 * \remarks Construct a new instance to encapsulate a new or altered set of mappings.
	 * Key mappings only traverse states in these paths, note they begin at "initial":
	 * 1. initial -> down -> repeat -> up
	 * 2. initial -> down -> up
	 */
	class KeyboardActionTranslator
	{
		//// Hard coded maximum extent of 128 mappings, this will enable us to use
		//// a plain old array with iterators and pointers that don't get invalidated.
		//inline static constexpr std::size_t MapBufSize{ 128 };
		//// (or a fancy std::array container)
		//std::array<CBActionMap, MapBufSize> m_mapBuf;
		////TODO use compile time sized arrays as buffers and constexpr "fill" functions that fill the bufs
		///// and return the count filled into the arrays. Doing that may enable a lot of code to be eliminated
		///// if used with pre-programmed mappings.
		//TODO for now the code typically uses the convention of returning a range, changing it will require some extensive work.
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

			TranslationPack tPack;

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
			//TODO bug here somewhere, see tests.

			// Get exclusivity group overtaken
			for(const auto cur : uniqueMatches)
			{
				tPack.OvertakenRequests.append_range(GetOvertakenTranslationResultsFor(cur));
			}
			// Adds maps with timer being reset (updated)
			transform(updateIndices, std::back_inserter(tPack.UpdateRequests), [&](const auto n)
			{
				return GetUpdateTranslationResultAt(n);
			});
			// Adds maps being key-repeat'd
			transform(repeatIndices, std::back_inserter(tPack.RepeatRequests), [&](const auto n)
			{
				return GetRepeatTranslationResultAt(n);
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
					results.emplace_back(TranslationResult{ .DoState = ButtonStateMgr::ActionState::KEYUP, .ButtonMapping = &elem, .Priority = PriorityMgr::PriorityState::NEXT_STATE });
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
		auto GetUpdateTranslationResultAt(const std::uint32_t ind) -> TranslationResult
		{
			return TranslationResult
			{
				.DoState = ButtonStateMgr::ActionState::INIT,
				.ButtonMapping = &m_mappings.at(ind),
				.Priority = PriorityMgr::PriorityState::UPDATE
			};
		}
		auto GetRepeatTranslationResultAt(const std::uint32_t ind)  -> TranslationResult
		{
			return TranslationResult
			{
				.DoState = ButtonStateMgr::ActionState::KEYREPEAT,
				.ButtonMapping = &m_mappings.at(ind),
				.Priority = PriorityMgr::PriorityState::REPEAT
			};
		}
		auto GetOvertakenTranslationResultsFor(const std::uint32_t currentIndex) -> std::vector<TranslationResult>
		{
			// TODO this might add duplicates of the same action, will need tested.
			using std::ranges::find_if;
			// Iterate through each matching mapping and find ones with an exclusivity grouping, and then add the rest of the grouping to the results with key-up
			const auto& currentMap = m_mappings[currentIndex];
			if (currentMap.ExclusivityGrouping)
			{
				std::vector<TranslationResult> results;
				auto& exGroupVecForCurrent = m_exGroupMap[*currentMap.ExclusivityGrouping];
				for (CBActionMap* p : exGroupVecForCurrent)
				{
					if (p != &currentMap)
					{
						if (p->LastAction.IsDown() || p->LastAction.IsRepeating())
						{
							results.emplace_back(TranslationResult
								{
									ButtonStateMgr::ActionState::KEYUP,
									p,
									PriorityMgr::PriorityState::EX_OVERTAKING
								});
						}
					}
				}
				return results;
			}
			return {};
		}
		//auto GetExGroupOvertaken(const CBActionMap& currentMapping) -> std::vector<TranslationResult>
		//{
		//	// TODO this might add duplicates of the same action, will need tested.
		//	using std::ranges::find_if;
		//	std::vector<TranslationResult> results;
		//	// Iterate through each matching mapping and find ones with an exclusivity grouping, and then add the rest of the grouping to the results with key-up
		//	if (currentMapping.ExclusivityGrouping)
		//	{
		//		auto& exGroupVecForCurrent = m_exGroupMap[*currentMapping.ExclusivityGrouping];
		//		for (CBActionMap* p : exGroupVecForCurrent)
		//		{
		//			if (p != &currentMapping)
		//			{
		//				if (p->LastAction.IsDown() || p->LastAction.IsRepeating())
		//					results.emplace_back(TranslationResult{ .DoDown = false, .DoUp = true, .DoRepeat = false, .DoReset = false, .ButtonMapping = p });
		//			}
		//		}
		//	}
		//	return results;
		//}
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
			const bool isButtonRepeat = buttonInfo.KeyRepeat;
			// Initial key-down case
			if (isButtonDown && isMapInit)
			{
				results.emplace_back(TranslationResult
					{
						.DoState = ButtonStateMgr::ActionState::KEYDOWN,
						.ButtonMapping = &currentMapping,
						.Priority = PriorityMgr::PriorityState::NEXT_STATE
					});
			}
			if ((isButtonDown || isButtonRepeat) && isMapDown)
			{
				results.emplace_back(TranslationResult
					{
						.DoState = ButtonStateMgr::ActionState::KEYREPEAT,
						.ButtonMapping = &currentMapping,
						.Priority = PriorityMgr::PriorityState::NEXT_STATE
					});
			}
			// Key-up case
			if (isButtonUp && (isMapDown || isMapRepeat))
			{
				results.emplace_back(TranslationResult
					{
						.DoState = ButtonStateMgr::ActionState::KEYUP,
						.ButtonMapping = &currentMapping,
						.Priority = PriorityMgr::PriorityState::NEXT_STATE
					});
			}
			// special repeat case
			//if (isButtonRepeat && isMapRepeat)
			//{
			//	results.emplace_back(TranslationResult
			//		{
			//			.DoState = ButtonStateMgr::ActionState::KEYREPEAT,
			//			.ButtonMapping = &currentMapping,
			//			.Priority = PriorityMgr::PriorityState::REPEAT
			//		});
			//}
		}
	};

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

	/**
	 * \brief Returns mappings for the controller button VK given.
	 * \param vk Controller button Virtual Keycode.
	 * \param mappingsList List of controller button to action mappings.
	 */
	inline
	auto GetMappingsMatchingVk(const int vk, std::vector<CBActionMap>& mappingsList) -> std::vector<CBActionMap*>
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
		for(std::uint32_t i{}; i < mappingsList.size(); ++i)
		{
			const auto& elem = mappingsList[i];
			if (elem.Vk == vk)
				buf.emplace_back(i);
		}
		return buf;
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
				const bool DoUpdate = (elem.LastAction.IsUp() && elem.LastAction.LastSentTime.IsElapsed()) && elem.UsesRepeat;
				const bool DoImmediate = elem.LastAction.IsUp() && !elem.UsesRepeat;
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
			const bool doesRepeat = elem.UsesRepeat;
			const bool isDown = elem.LastAction.IsDown();
			const bool isRepeating = elem.LastAction.IsRepeating();
			const bool downOrRepeat = isDown || isRepeating;
			const bool isElapsed = elem.LastAction.LastSentTime.IsElapsed();
			if (doesRepeat && downOrRepeat && isElapsed)
			{
				buf.emplace_back(i);
			}
		}
		return buf;
	}

	inline
	auto GetUniqueMatches(const std::vector<std::uint32_t> existing, const std::vector<std::uint32_t> toAdd) -> std::vector<std::uint32_t>
	{
		using std::ranges::find, std::ranges::end, std::ranges::begin, std::ranges::transform;
		// TODO use some of the algo header funcs, possibly set_intersection or merge or similar
		std::vector<std::uint32_t> uniqueMatchResult;
		// Don't add existing indices to a set or anything, cpu arch will speed up iterating an array 1-100x iterating a r-b tree.
		//const std::set tempSet(std::ranges::begin(existingIndices), std::ranges::end(existingIndices));
		for (std::uint32_t i{}; i < toAdd.size(); ++i)
		{
			const auto e = toAdd[i];
			if (find(existing, e) == end(existing))
			{
				uniqueMatchResult.emplace_back(e);
			}
		}
		return uniqueMatchResult;
	}

	//inline
	//auto GetUniqueMatches(const std::vector<std::uint32_t> existing, const std::vector<std::uint32_t> toAdd) -> std::vector<std::uint32_t>
	//{
	//	using std::ranges::find, std::ranges::end, std::ranges::begin, std::ranges::transform;
	//	std::vector<std::uint32_t> uniqueMatchResult;
	//	// Don't add existing indices to a set or anything, cpu arch will speed up iterating an array 1-100x iterating a r-b tree.
	//	//const std::set tempSet(std::ranges::begin(existingIndices), std::ranges::end(existingIndices));
	//	for(std::uint32_t i{}; i < toAdd.size(); ++i)
	//	{
	//		const auto e = toAdd[i];
	//		if(find(existing, e) == end(existing))
	//		{
	//			uniqueMatchResult.emplace_back(e);
	//		}
	//	}
	//	return uniqueMatchResult;
	//}
}

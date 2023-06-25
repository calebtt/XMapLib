#pragma once
#include "LibIncludes.h"
#include "CustomTypes.h"

#include "ControllerStateUpdateWrapper.h"
#include "KeyboardTranslationResult.h"
#include "KeyboardActionTranslator.h"
#include "LegacyApiFunctions.h"

/*
 *	Note: There are some static sized arrays used here with capacity defined in customtypes.
 */

namespace sds
{
	template<typename Poller_t>
	concept IsInputPoller = requires(Poller_t & t)
	{
		{ t.GetUpdatedState() };
		{ t.GetUpdatedState() } -> std::convertible_to<ControllerStateUpdateWrapper<>>;
	};

	/*
	 *	NOTE: Testing these functions may be quite easy, pass a single CBActionMap in various states to each of these functions,
	 *	and if more than one TranslationResult is produced (aside from the reset translation), then it would obviously be in error.
	 */

	/**
	 * \brief For a single mapping, search the controller state update buffer and produce a TranslationResult appropriate to the current mapping state and controller state.
	 * \param updatesWrapper Wrapper class containing the results of a controller state update polling.
	 * \param singleButton The mapping type for a single virtual key of the controller.
	 * \returns Optional, <c>TranslationResult</c>
	 */
	[[nodiscard]]
	inline
	auto GetButtonTranslationForInitialToDown(const ControllerStateUpdateWrapper<>& updatesWrapper, CBActionMap& singleButton) noexcept -> std::optional<TranslationResult>
	{
		using std::ranges::find, std::ranges::end;
		if (singleButton.LastAction.IsInitialState())
		{
			const auto downResults = updatesWrapper.GetDownVirtualKeycodesRange();
			const auto findResult = find(downResults, singleButton.ButtonVirtualKeycode);
			// If VK *is* found in the down list, create the down translation.
			if(findResult != end(downResults))
				return GetInitialKeyDownTranslationResult(singleButton);
		}
		return {};
	}

	[[nodiscard]]
	inline
	auto GetButtonTranslationForDownToRepeat(const ControllerStateUpdateWrapper<>& updatesWrapper, CBActionMap& singleButton) noexcept -> std::optional<TranslationResult>
	{
		using std::ranges::find, std::ranges::end;
		const bool isDownAndUsesRepeat = singleButton.LastAction.IsDown() && (singleButton.UsesInfiniteRepeat || singleButton.SendsFirstRepeatOnly);
		if (isDownAndUsesRepeat && singleButton.LastAction.DelayBeforeFirstRepeat.IsElapsed())
		{
			const auto downResults = updatesWrapper.GetDownVirtualKeycodesRange();
			const auto findResult = find(downResults, singleButton.ButtonVirtualKeycode);
			// If VK *is* found in the down list, create the repeat translation.
			if (findResult != end(downResults))
				return GetRepeatTranslationResult(singleButton);
		}
		return {};
	}

	[[nodiscard]]
	inline
	auto GetButtonTranslationForRepeatToRepeat(const ControllerStateUpdateWrapper<>& updatesWrapper, CBActionMap& singleButton) noexcept -> std::optional<TranslationResult>
	{
		using std::ranges::find, std::ranges::end;
		const bool isRepeatAndUsesInfinite = singleButton.LastAction.IsRepeating() && singleButton.UsesInfiniteRepeat;
		if (isRepeatAndUsesInfinite && singleButton.LastAction.LastSentTime.IsElapsed())
		{
			const auto downResults = updatesWrapper.GetDownVirtualKeycodesRange();
			const auto findResult = find(downResults, singleButton.ButtonVirtualKeycode);
			// If VK *is* found in the down list, create the repeat translation.
			if (findResult != end(downResults))
				return GetRepeatTranslationResult(singleButton);
		}
		return {};
	}

	[[nodiscard]]
	inline
	auto GetButtonTranslationForDownOrRepeatToUp(const ControllerStateUpdateWrapper<>& updatesWrapper, CBActionMap& singleButton) noexcept -> std::optional<TranslationResult>
	{
		using std::ranges::find, std::ranges::end;
		if (singleButton.LastAction.IsDown() || singleButton.LastAction.IsRepeating())
		{
			const auto downResults = updatesWrapper.GetDownVirtualKeycodesRange();
			const auto findResult = find(downResults, singleButton.ButtonVirtualKeycode);
			// If VK is not found in the down list, create the up translation.
			if(findResult == end(downResults))
				return GetKeyUpTranslationResult(singleButton);
		}
		return {};
	}

	// This is the reset translation
	[[nodiscard]]
	inline
	auto GetButtonTranslationForUpToInitial(CBActionMap& singleButton) noexcept -> std::optional<TranslationResult>
	{
		using std::ranges::find, std::ranges::end;
		// if the timer has elapsed, update back to the initial state.
		if(singleButton.LastAction.IsUp() && singleButton.LastAction.LastSentTime.IsElapsed())
		{
			return GetResetTranslationResult(singleButton);
		}
		return {};
	}

	// Poller builds the translation results with the mapping functionality.
	class KeyboardPollerControllerLegacy
	{
		using MappingVector_t = std::vector<CBActionMap>;
		std::vector<CBActionMap> m_mappings;
	public:
		KeyboardPollerControllerLegacy() = delete;

		explicit KeyboardPollerControllerLegacy(MappingVector_t&& keyMappings )
		: m_mappings(std::move(keyMappings))
		{ }
		explicit KeyboardPollerControllerLegacy(const MappingVector_t& keyMappings)
			: m_mappings(keyMappings)
		{ }
	public:
		[[nodiscard]]
		auto operator()(const ControllerStateUpdateWrapper<>& stateUpdate) noexcept -> std::vector<TranslationResult>
		{
			return GetUpdatedState(stateUpdate);
		}
		[[nodiscard]]
		auto GetUpdatedState(const ControllerStateUpdateWrapper<>& stateUpdate) noexcept -> std::vector<TranslationResult>
		{
			std::vector<TranslationResult> translations;
			for (auto& mapping : m_mappings)
			{
				if (const auto upToInitial = GetButtonTranslationForUpToInitial(mapping))
				{
					translations.emplace_back(*upToInitial);
					continue;
				}
				if (const auto initialToDown = GetButtonTranslationForInitialToDown(stateUpdate, mapping))
				{
					translations.emplace_back(*initialToDown);
					continue;
				}
				if (const auto downToFirstRepeat = GetButtonTranslationForDownToRepeat(stateUpdate, mapping))
				{
					translations.emplace_back(*downToFirstRepeat);
					continue;
				}
				if (const auto repeatToRepeat = GetButtonTranslationForRepeatToRepeat(stateUpdate, mapping))
				{
					translations.emplace_back(*repeatToRepeat);
					continue;
				}
				if (const auto repeatToUp = GetButtonTranslationForDownOrRepeatToUp(stateUpdate, mapping))
				{
					translations.emplace_back(*repeatToUp);
				}
			}
			return translations;
		}
	};

}
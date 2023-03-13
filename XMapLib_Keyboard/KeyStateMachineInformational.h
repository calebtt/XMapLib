#pragma once
#include "LibIncludes.h"
#include "KeyboardPoller.h"
#include "ControllerSideDetails.h"
#include <iostream>
#include <chrono>
#include <optional>

namespace sds
{
	// Concept for keymap info type, provides things like GetLastAction() and IsExclusivityBlocked()
	//template<typename T>
	//concept KMInformational_c = requires (T & t)
	//{
	//	// Information
	//	//{ t.GetMapBuffer() } -> std::ranges::range;
	//	{ t.GetLastAction() } -> ControllerButtonStateData::ActionType;
	//	{ t.GetButtonVK() } -> int;
	//	{ t.IsExclusivityBlocked() } -> std::convertible_to<bool>;
	//	// Data members
	//	{ t.ControllerButtonState };
	//	{ t.KeymapData };
	//	{ t.ControllerButton };
	//};
	/**
	 * \brief Instances are attached to CBTAM mappings, It operates a state machine for the case of
	 * controller button to keyboard button mappings. Just feed it ControllerStateWrapper structs.
	 */
	 struct KeyStateMachineInformational
	 {
		using Delay_t = ControllerButtonStateData::Delay_t;
		using InpType = ControllerButtonStateData::ActionType;
		InpType m_lastAction{};
		public:
		ControllerButtonStateData* m_buttonStateData{};
		ControllerButtonData* m_controllerButtonVK{};
		ControllerToKeyMapData* m_keyMapData{};
		bool m_isInExclusivityGroup{ false };
		bool m_isInExclusivityNoUpdateGroup{ false };
		std::function<bool()> IsBlockedByExclusiveGroup;
		public:
		explicit KeyStateMachineInformational(
			ControllerButtonStateData &cbsd, 
			ControllerButtonData &cbd, 
			ControllerToKeyMapData &ctkmd,
			const bool isInExclusivityGroup,
			const bool isInExclusivityNoUpdateGroup,
			auto&& isExclusivityBlockedFn
		)
			: m_buttonStateData(&cbsd),
		m_controllerButtonVK(&cbd),
		m_keyMapData(&ctkmd),
		m_isInExclusivityGroup(isInExclusivityGroup),
		m_isInExclusivityNoUpdateGroup(isInExclusivityNoUpdateGroup),
		IsBlockedByExclusiveGroup(isExclusivityBlockedFn)
		{

		}
	 public:
	 	/**
	 	 * \brief Tests the stroke against the state machine to see if a key-down should be sent.
	 	 * \param stroke info about controller key-press event.
	 	 * \returns true if key-down should be sent.
	 	 */
	 	auto TestDown(const ControllerStateWrapper& stroke) noexcept
	 		-> bool
	 	{
	 		constexpr auto noneType = InpType::NONE;
	 		constexpr auto upType = InpType::KEYUP;

			const auto lastAction = GetLastAction();
			const auto controllerButtonVK = GetControllerVK();

	 		// If this cbtam is being asked to do down/repeat/up.
			const bool AskDown = stroke.KeyDown && stroke.VirtualKey == controllerButtonVK;
			//const bool IsOvertaking = m_currentMapping.IsExclusivityBlocked();

	 		// If the current state will allow it to actually do down/repeat/up.
	 		const bool DoDown = ((lastAction == upType) || (lastAction == noneType)) && AskDown;
			// Test for being in exclusive no update group and currently excluded by another pressed button.
			const bool IsExcluded = m_isInExclusivityNoUpdateGroup && IsBlockedByExclusiveGroup();
			const bool SendDown = DoDown && !IsExcluded;
			if (SendDown)
				m_lastAction = InpType::KEYDOWN;
	 		return SendDown;
	 	}

	 	/**
	 	 * \brief Tests the stroke against the state machine to see if a key-up should be sent.
	 	 * \param stroke info about controller key-press event.
	 	 * \returns true if key-up should be sent.
	 	 */
	 	auto TestUp(const ControllerStateWrapper& stroke) noexcept
	 		-> bool
	 	{
	 		constexpr auto downType = InpType::KEYDOWN;
	 		constexpr auto repeatType = InpType::KEYREPEAT;

			const auto lastAction = GetLastAction();
			const auto controllerButtonVK = GetControllerVK();

	 		// If this cbtam is being asked to do down/repeat/up.
			const bool AskUp = stroke.KeyUp && stroke.VirtualKey == controllerButtonVK;

	 		// If the current state will allow it to actually do down/repeat/up.
	 		const bool DoUp = (lastAction == downType || lastAction == repeatType) && AskUp;

			if (DoUp)
				m_lastAction = InpType::KEYUP;

	 		// Then do down/repeat/up.
	 		return DoUp;
	 	}

	 	/**
	 	 * \brief Tests the stroke against the state machine to see if a key-repeat should be sent.
	 	 * \param stroke info about controller key-press event.
	 	 * \returns true if key-up should be sent.
	 	 */
	 	auto TestRepeat(const ControllerStateWrapper& stroke) noexcept
	 		-> bool
	 	{
			// First run the update check.
			ResetForRepeat(stroke);

	 		constexpr auto downType = InpType::KEYDOWN;
	 		constexpr auto repeatType = InpType::KEYREPEAT;

			const auto lastAction = GetLastAction();
			const auto controllerButtonVK = GetControllerVK();
			const bool usesRepeat = GetUsesRepeat();

	 		// If this cbtam is being asked to do down/repeat/up.
			const bool AskRepeat = stroke.KeyRepeat && stroke.VirtualKey == controllerButtonVK;

	 		// If the current state will allow it to actually do down/repeat/up.
			const bool DoRepeat = (lastAction == repeatType || lastAction == downType) && AskRepeat && usesRepeat;
			if (DoRepeat)
				m_lastAction = InpType::KEYREPEAT;

	 		return DoRepeat;
	 	}
	 private:
	    /**
	     * \brief This class must be updated when the key update loop resets the timer (and thus state).
	     */
	    auto ResetForRepeat(const ControllerStateWrapper& stroke) -> void
	 	{
			auto& cbState = GetRepeatTimer();

			if(cbState.IsElapsed() && GetUsesRepeat())
			{
				const bool isLastRepeating = m_lastAction == InpType::KEYREPEAT;
				const bool isLastKeyup = m_lastAction == InpType::KEYUP;
				if(stroke.KeyRepeat && isLastRepeating)
				{
					cbState.Reset(GetDelayAfterRepeat().count());
				}
				else if(isLastKeyup)
				{
					cbState.Reset(GetDelayBeforeRepeat().count());
					m_lastAction = InpType::NONE;
				}
			}
	 	}

		[[nodiscard]]
		auto GetLastAction() const noexcept -> InpType
	    {
		    return m_buttonStateData->LastAction;
	    }

		[[nodiscard]]
		auto GetControllerVK() const noexcept -> int
	    {
		    return m_controllerButtonVK->VK;
	    }
		[[nodiscard]]
		auto GetUsesRepeat() const noexcept -> bool
	    {
			return m_keyMapData->UsesRepeat;
	    }
		[[nodiscard]]
		auto GetRepeatTimer() const noexcept -> Delay_t
	    {
			return m_buttonStateData->LastSentTime;
	    }
		[[nodiscard]]
		auto GetDelayBeforeRepeat() const noexcept -> std::chrono::milliseconds
	    {
			return m_keyMapData->DelayBeforeRepeatActivation;
	    }
		[[nodiscard]]
		auto GetDelayAfterRepeat() const noexcept -> std::chrono::milliseconds
	    {
			return m_keyMapData->DelayAfterRepeatActivation;
	    }
		// TODO the issue of where exactly the information to do IsOvertaking exists and the logic should be.

	 	///**
	 	// * \brief Check to see if an exclusivity grouping has been pressed already
	 	// * \param detail Newest element being set to keydown state
	 	// * \return one of three states, to be std::visit'd for what to do in that state
	 	// */
	 	//[[nodiscard]]
	 	//auto IsOvertaking(const ControllerStateWrapper& detail) noexcept
	 	//	-> bool
	 	//{
	 	//	using std::ranges::all_of, std::ranges::find, std::ranges::find_if, std::ranges::begin, std::ranges::end;
	 	//	// Get copy of range to pointers to all mappings in existence.
	 	//	const auto mapBuffer = detail.GetMapBuffer();
	 	//	// Get copy of pointers to all mappings in the same exclusivity grouping as this mapping.
	 	//	const auto groupedBuffer = std::ranges::views::transform(mapBuffer, [exGroup = m_currentMapping.KeymapData.ExclusivityGrouping](const auto& elem)
	 	//		{
	 	//			return elem->KeymapData.ExclusivityGrouping == exGroup;
	 	//		});
	 	//	const auto groupedNoUpdateBuffer = std::ranges::views::transform(mapBuffer, [exGroup = m_currentMapping.KeymapData.ExclusivityNoOvertakingGrouping](const auto& elem)
	 	//		{
	 	//			return elem->KeymapData.ExclusivityNoOvertakingGrouping == exGroup;
	 	//		});

	 	//	// If one or the other has members, assert that they don't BOTH have members as this is obviously a problem.
	 	//	if (!groupedBuffer.empty() || !groupedNoUpdateBuffer.empty())
	 	//		assert(groupedBuffer.empty() ^ groupedNoUpdateBuffer.empty());

	 	//	const auto groupedResult = GetGroupedOvertaken(detail, groupedBuffer);
	 	//	const auto groupedNoUpdateResult = GetGroupedOvertaken(detail, groupedNoUpdateBuffer);
	 	//	if (groupedResult)
	 	//		return OvertakingKeyDown_t{ groupedResult.value(), &m_currentMapping };
	 	//	if (groupedNoUpdateResult)
	 	//		return NoUpdate_t{};
	 	//	return NormalKeyDown_t{ &m_currentMapping };
	 	//}

	 	///**
	 	// * \brief Checks the exclusivity group members for a key being overtaken by the newly pressed key.
	 	// * \param detail button newly pressed
	 	// * \param groupedBuffer buffer holding the state of all keys at present within the grouping
	 	// * \return optional, pointer to cbtam being overtaken
	 	// */
	 	//auto GetGroupedOvertaken(
	 	//	const KMInfo_t& detail,
	 	//	const auto& groupedBuffer) -> std::optional<KMInfo_t*>
	 	//{
	 	//	using std::ranges::find_if, std::ranges::end;
	 	//	// If exclusivity with update grouping has some other members...
	 	//	if (!groupedBuffer.empty())
	 	//	{
	 	//		auto IsGroupedBtnPressedFn = [&](const KMInfo_t* groupedButtonElem)
	 	//		{
	 	//			const auto elemState = groupedButtonElem->ControllerButtonState.LastAction;
	 	//			const auto elemButtonVK = groupedButtonElem->ControllerButton.VK;
	 	//			constexpr auto DownType = InpType::KEYDOWN;
	 	//			constexpr auto RepeatType = InpType::KEYREPEAT;
	 	//			const bool isOtherButtonPressed = elemState == DownType || elemState == RepeatType;
	 	//			// this is the case where the key we're testing for is the same key we're investigating
	 	//			const bool isSameKey = elemButtonVK == detail.ControllerButton.VK;
	 	//			if (!isSameKey)
	 	//			{
	 	//				return isOtherButtonPressed;
	 	//			}
	 	//			return false;
	 	//		};

	 	//		const auto mpit = find_if(groupedBuffer, IsGroupedBtnPressedFn);
	 	//		if (mpit == end(groupedBuffer))
	 	//		{
	 	//			return {};
	 	//		}
	 	//		return *mpit;
	 	//	}
	 	//	return {};
	 	//}

	 };
}

#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include <iostream>
#include <chrono>
#include <optional>

namespace sds
{
	/**
	 * \brief Instances are attached to CBTAM mappings, It operates a state machine for the case of
	 * controller button to keyboard button mappings. Just feed it ControllerStateWrapper structs.
	 */
	struct KeyboardTranslator
	{
		using InpType = ControllerButtonStateData::ActionType;
		using CBTAM = ControllerButtonToActionMap<>;

		// These are *class* states when processing a keystroke.
		struct OvertakingKeyDown_t
		{
			CBTAM* pOvertaken{};
			CBTAM* pCurrentKeyState{};
			OvertakingKeyDown_t(CBTAM* overtakenBtn, CBTAM* newPressBtn)
				: pOvertaken(overtakenBtn), pCurrentKeyState(newPressBtn) { }
			void operator()(const InpType downType) const noexcept
			{
				assert(pCurrentKeyState != nullptr);
				assert(pOvertaken != nullptr);
				// send up
				KeyUp_t kut{ pOvertaken };
				kut(InpType::KEYUP);
				// send down
				NormalKeyDown_t t{ pCurrentKeyState };
				t(downType);
			}
		};
		struct NoUpdate_t
		{
			void operator()([[maybe_unused]] const InpType downType) const noexcept { /* nothing */ }
		};
		struct NormalKeyDown_t
		{
			CBTAM* pCurrentKeyState{};
			NormalKeyDown_t(CBTAM* newPressBtn)
			: pCurrentKeyState(newPressBtn) { }
			void operator()(const InpType downType) const noexcept
			{
				using std::chrono::duration_cast;
				using timeUnit_t = ControllerToKeyMapData::Cms_t;
				assert(pCurrentKeyState != nullptr);
				// send downtype
				pCurrentKeyState->MappedActionsArray[downType]();
				// update LastAction
				pCurrentKeyState->ControllerButtonState.LastAction = downType;
				// and last sent time
				pCurrentKeyState->ControllerButtonState.LastSentTime.Reset(duration_cast<timeUnit_t>(pCurrentKeyState->KeymapData.DelayBeforeRepeatActivation).count());
			}
		};
		struct KeyUp_t
		{
			CBTAM* pCurrentKeyState{};
			KeyUp_t(CBTAM* newPressBtn) : pCurrentKeyState(newPressBtn) { }
			void operator()([[maybe_unused]] const InpType upType) const noexcept
			{
				using std::chrono::duration_cast;
				using timeUnit_t = ControllerToKeyMapData::Cms_t;
				assert(pCurrentKeyState != nullptr);
				auto& curState = pCurrentKeyState->ControllerButtonState;
				// send keyup
				pCurrentKeyState->MappedActionsArray[InpType::KEYUP]();
				// update LastAction
				curState.LastAction = InpType::KEYUP;
				curState.LastSentTime.Reset(duration_cast<timeUnit_t>(pCurrentKeyState->KeymapData.DelayBeforeRepeatActivation).count());
			}
		};
		struct RepeatKeyDown_t
		{
			CBTAM* pCurrentKeyState{};
			RepeatKeyDown_t(CBTAM* newPressBtn)
				: pCurrentKeyState(newPressBtn) { }
			void operator()(const InpType downType) const noexcept
			{
				// This is repeated here because it has to use a different duration on the timer.
				// todo extract into a function
				using std::chrono::duration_cast;
				using timeUnit_t = ControllerToKeyMapData::Cms_t;
				assert(pCurrentKeyState != nullptr);
				// send downtype
				pCurrentKeyState->MappedActionsArray[downType]();
				// update LastAction
				pCurrentKeyState->ControllerButtonState.LastAction = downType;
				// and last sent time
				pCurrentKeyState->ControllerButtonState.LastSentTime.Reset(duration_cast<timeUnit_t>(pCurrentKeyState->KeymapData.DelayAfterRepeatActivation).count());
			}
		};
	public:
		KeyboardSettingsPack m_ksp;
		CBTAM m_currentMapping;
	public:
		explicit KeyboardTranslator(CBTAM&& mapToOperateOn, const KeyboardSettingsPack& ksp = {})
			: m_ksp(ksp), m_currentMapping(std::move(mapToOperateOn))
		{

		}
	public:
		/**
		 * \brief If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--otherwise reset it immediately.
		 * The LastAction and LastSentTime get updated here.
		 * \param mapBuffer container holding all CBTAM pointers.
		 */
		static
		void KeyUpdateLoop(const std::vector<CBTAM*> mapBuffer) noexcept
		{
			using std::chrono::duration_cast, std::chrono::microseconds;
			//If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--
			//otherwise reset it immediately.
			for (const auto& elem : mapBuffer)
			{
				auto &cbData = elem->ControllerButtonState;
				const auto mapData = elem->KeymapData;
				const bool DoUpdate = (cbData.LastAction == InpType::KEYUP && cbData.LastSentTime.IsElapsed()) && mapData.UsesRepeat;
				const bool DoImmediate = cbData.LastAction == InpType::KEYUP && !mapData.UsesRepeat;
				if (DoUpdate || DoImmediate)
				{
					cbData.LastAction = InpType::NONE;
					cbData.LastSentTime.Reset(duration_cast<microseconds>(elem->KeymapData.DelayAfterRepeatActivation).count());
				}
			}
		}

		/**
		 * \brief Checks each <b>ControllerButtonToActionMap</b>'s <c>LastSentTime</c> timer for being elapsed,
		 * and if so, sends the repeat keypress (if key repeat behavior is enabled for the map).
		 * \param mapBuffer container holding all CBTAM pointers.
		 */
		static
		void KeyRepeatLoop(const std::vector<CBTAM*> mapBuffer) noexcept
		{
			using AT = InpType;
			for (const auto& w : mapBuffer)
			{
				const bool usesRepeat = w->KeymapData.UsesRepeat;
				const auto lastAction = w->ControllerButtonState.LastAction;
				if (usesRepeat && (lastAction == AT::KEYDOWN || lastAction == AT::KEYREPEAT))
				{
					if (w->ControllerButtonState.LastSentTime.IsElapsed())
					{
						RepeatKeyDown_t t{ w };
						t(InpType::KEYREPEAT);
					}
				}
			}
		}

		/**
		 * \brief Entry point for key-down state machine logic. Manages explicitly the requested action.
		 * Then performs the KeyRepeat loop and the KeyUpdate loop (resets keys for use again).
		 * \param stroke info about controller key-press event.
		 */
		void DoDown(const ControllerStateWrapper& stroke) noexcept
		{
			// Start with key-update loop and key-repeat loop
			KeyUpdateLoop(m_currentMapping.GetThisBuffer());

			constexpr auto noneType = InpType::NONE;
			constexpr auto downType = InpType::KEYDOWN;
			constexpr auto upType = InpType::KEYUP;

			const auto lastAction = m_currentMapping.ControllerButtonState.LastAction;

			// If this cbtam is being asked to do down/repeat/up.
			const bool AskDown = stroke.KeyDown && stroke.VirtualKey == m_currentMapping.ControllerButton.VK;

			// If the current state will allow it to actually do down/repeat/up.
			const bool DoDown = ((lastAction == upType) || (lastAction == noneType)) && AskDown;

			// Then do down/repeat/up.
			if (DoDown)
			{
				auto overtakingAction = IsOvertaking(m_currentMapping);
				std::visit([&](auto& t) { t(downType); }, overtakingAction);
			}
			KeyRepeatLoop(m_currentMapping.GetThisBuffer());
		}

		/**
		 * \brief For key-up state machine logic. Manages explicitly the requested action.
		 * Then performs the KeyRepeat loop and the KeyUpdate loop (resets keys for use again).
		 * \param stroke info about controller key-press event.
		 */
		void DoUp(const ControllerStateWrapper& stroke) noexcept
		{
			// Start with key-update loop and key-repeat loop
			KeyUpdateLoop(m_currentMapping.GetThisBuffer());

			constexpr auto downType = InpType::KEYDOWN;
			constexpr auto repeatType = InpType::KEYREPEAT;

			const auto lastAction = m_currentMapping.ControllerButtonState.LastAction;

			// If this cbtam is being asked to do down/repeat/up.
			const bool AskUp = stroke.KeyUp && stroke.VirtualKey == m_currentMapping.ControllerButton.VK;

			// If the current state will allow it to actually do down/repeat/up.
			const bool DoUp = (lastAction == downType || lastAction == repeatType) && AskUp;

			// Then do down/repeat/up.
			if (DoUp)
			{
				KeyUp_t k(&m_currentMapping);
				k(InpType::KEYUP);
			}
		}
		
		/**
		 * \brief For key-repeat state machine logic. Manages explicitly the requested action.
		 *  Also performs the KeyRepeat loop and the KeyUpdate loop (resets keys for use again).
		 * \param stroke info about controller key-press event.
		 */
		void DoRepeat(const ControllerStateWrapper& stroke) noexcept
		{
			// Start with key-update loop and key-repeat loop
			KeyUpdateLoop(m_currentMapping.GetThisBuffer());

			constexpr auto downType = InpType::KEYDOWN;
			constexpr auto repeatType = InpType::KEYREPEAT;

			const auto lastAction = m_currentMapping.ControllerButtonState.LastAction;

			// If this cbtam is being asked to do down/repeat/up.
			const bool AskRepeat = stroke.KeyRepeat && stroke.VirtualKey == m_currentMapping.ControllerButton.VK;

			// If the current state will allow it to actually do down/repeat/up.
			const bool DoRepeat = (lastAction == repeatType || lastAction == downType) && AskRepeat && m_currentMapping.KeymapData.UsesRepeat;

			// Then do down/repeat/up.
			if (DoRepeat)
			{
				//Repeat'd key cannot be overtaking, only a newly key-down'd key.
				NormalKeyDown_t t{ &m_currentMapping };
				t(InpType::KEYREPEAT);
			}
			KeyRepeatLoop(m_currentMapping.GetThisBuffer());
		}
	private:

		/**
		 * \brief Check to see if an exclusivity grouping has been pressed already
		 * \param detail Newest element being set to keydown state
		 * \return one of three states, to be std::visit'd for what to do in that state
		 */
		[[nodiscard]]
		auto IsOvertaking(const CBTAM& detail) noexcept
			-> std::variant<OvertakingKeyDown_t, NoUpdate_t, NormalKeyDown_t>
		{
			using std::ranges::all_of, std::ranges::find, std::ranges::find_if, std::ranges::begin, std::ranges::end;
			// Get copy of range to pointers to all mappings in existence.
			const auto mapBuffer = detail.GetThisBuffer();
			// Get copy of pointers to all mappings in the same exclusivity grouping as this mapping.
			const auto groupedBuffer = std::ranges::views::transform(mapBuffer, [exGroup = m_currentMapping.KeymapData.ExclusivityGrouping](const auto& elem)
				{
					return elem->KeymapData.ExclusivityGrouping == exGroup;
				});
			const auto groupedNoUpdateBuffer = std::ranges::views::transform(mapBuffer, [exGroup = m_currentMapping.KeymapData.ExclusivityNoOvertakingGrouping](const auto& elem)
				{
					return elem->KeymapData.ExclusivityNoOvertakingGrouping == exGroup;
				});

			// If one or the other has members, assert that they don't BOTH have members as this is obviously
			// a problem.
			if (!groupedBuffer.empty() || !groupedNoUpdateBuffer.empty())
				assert(groupedBuffer.empty() ^ groupedNoUpdateBuffer.empty());

			const auto groupedResult = GetGroupedOvertaken(detail, groupedBuffer);
			const auto groupedNoUpdateResult = GetGroupedOvertaken(detail, groupedNoUpdateBuffer);
			if (groupedResult)
				return OvertakingKeyDown_t{ groupedResult.value(), &m_currentMapping };
			if (groupedNoUpdateResult)
				return NoUpdate_t{};
			return NormalKeyDown_t{ &m_currentMapping };
		}

		/**
		 * \brief Checks the exclusivity group members for a key being overtaken by the newly pressed key.
		 * \param detail button newly pressed
		 * \param groupedBuffer buffer holding the state of all keys at present within the grouping
		 * \return optional, pointer to cbtam being overtaken
		 */
		auto GetGroupedOvertaken(
			const CBTAM& detail,
			const auto& groupedBuffer) -> std::optional<CBTAM*>
		{
			using std::ranges::find_if, std::ranges::end;
			// If exclusivity with update grouping has some other members...
			if (!groupedBuffer.empty())
			{
				auto IsGroupedBtnPressedFn = [&](const CBTAM* groupedButtonElem)
				{
					const auto elemState = groupedButtonElem->ControllerButtonState.LastAction;
					const auto elemButtonVK = groupedButtonElem->ControllerButton.VK;
					constexpr auto DownType = InpType::KEYDOWN;
					constexpr auto RepeatType = InpType::KEYREPEAT;
					const bool isOtherButtonPressed = elemState == DownType || elemState == RepeatType;
					// this is the case where the key we're testing for is the same key we're investigating
					const bool isSameKey = elemButtonVK == detail.ControllerButton.VK;
					if (!isSameKey)
					{
						return isOtherButtonPressed;
					}
					return false;
				};

				const auto mpit = find_if(groupedBuffer, IsGroupedBtnPressedFn);
				if (mpit == end(groupedBuffer))
				{
					return {};
				}
				return *mpit;
			}
			return {};
		}

	};
}

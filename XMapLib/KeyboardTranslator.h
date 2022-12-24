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
	/// <summary>
	///	Holds the functions used for mapping button -> keystroke.
	///	App-specific logic in the context of a generic controller button -> action library.
	///	TODO this may be moved to it's own project (app-specific stuff project).
	/// </summary>
	struct KeyboardTranslator
	{
		using InpType = ControllerButtonStateData::ActionType;
		// These are *class* states when processing a keystroke.
		struct OvertakingKeyDown_t
		{
			ControllerButtonToActionMap* pOvertaken{};
			ControllerButtonToActionMap* pCurrentKeyState{};
			OvertakingKeyDown_t(ControllerButtonToActionMap* overtakenBtn, ControllerButtonToActionMap* newPressBtn)
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
			ControllerButtonToActionMap* pCurrentKeyState{};
			NormalKeyDown_t(ControllerButtonToActionMap* newPressBtn)
			: pCurrentKeyState(newPressBtn) { }
			void operator()(const InpType downType) const noexcept
			{
				using std::chrono::duration_cast, std::chrono::microseconds;
				assert(pCurrentKeyState != nullptr);
				// send downtype
				pCurrentKeyState->MappedActionsArray[downType]();
				// update LastAction
				pCurrentKeyState->ControllerButtonState.LastAction = downType;
				// and last sent time
				pCurrentKeyState->ControllerButtonState.LastSentTime.Reset(duration_cast<microseconds>(pCurrentKeyState->KeymapData.DelayBeforeRepeatActivation).count());
			}
		};
		struct KeyUp_t
		{
			ControllerButtonToActionMap* pCurrentKeyState{};
			KeyUp_t(ControllerButtonToActionMap* newPressBtn) : pCurrentKeyState(newPressBtn) { }
			void operator()([[maybe_unused]] const InpType upType) const noexcept
			{
				using std::chrono::duration_cast, std::chrono::microseconds;
				assert(pCurrentKeyState != nullptr);
				auto& curState = pCurrentKeyState->ControllerButtonState;
				// send keyup
				pCurrentKeyState->MappedActionsArray[InpType::KEYUP]();
				// update LastAction
				curState.LastAction = InpType::KEYUP;
				curState.LastSentTime.Reset(duration_cast<microseconds>(pCurrentKeyState->KeymapData.DelayBeforeRepeatActivation).count());
			}
		};
		struct RepeatKeyDown_t
		{
			ControllerButtonToActionMap* pCurrentKeyState{};
			RepeatKeyDown_t(ControllerButtonToActionMap* newPressBtn)
				: pCurrentKeyState(newPressBtn) { }
			void operator()(const InpType downType) const noexcept
			{
				// This is repeated here because it has to use a different duration on the timer.
				// todo extract into a function
				using std::chrono::duration_cast, std::chrono::microseconds;
				assert(pCurrentKeyState != nullptr);
				// send downtype
				pCurrentKeyState->MappedActionsArray[downType]();
				// update LastAction
				pCurrentKeyState->ControllerButtonState.LastAction = downType;
				// and last sent time
				pCurrentKeyState->ControllerButtonState.LastSentTime.Reset(duration_cast<microseconds>(pCurrentKeyState->KeymapData.DelayAfterRepeatActivation).count());
			}
		};
	public:
		KeyboardSettingsPack m_ksp;
		ControllerButtonToActionMap* m_currentMapping{};
	public:
		explicit KeyboardTranslator(ControllerButtonToActionMap& mapToOperateOn, const KeyboardSettingsPack& ksp = {})
			: m_ksp(ksp), m_currentMapping(&mapToOperateOn)
		{
			assert(m_currentMapping != nullptr);
		}

		/**
		 * \brief If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--otherwise reset it immediately.
		 * The LastAction and LastSentTime get updated here.
		 * \param mapBuffer container holding all CBTAM pointers.
		 */
		static
		void KeyUpdateLoop(const std::vector<ControllerButtonToActionMap*> &mapBuffer) noexcept
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
		void KeyRepeatLoop(const std::vector<ControllerButtonToActionMap*>& mapBuffer) noexcept
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
		 * \brief Entry point for keypress state machine logic. Manages explicitly the requested action,
		 * normally keydown/keyrepeat/keyup. Then performs the KeyRepeat loop and the KeyUpdate loop (resets keys for use again).
		 * \param stroke info about controller key-press event.
		 */
		void Normal(const ControllerStateWrapper& stroke) noexcept
		{
			// Start with key-update loop and key-repeat loop
			KeyUpdateLoop(m_currentMapping->GetThisBuffer());

			constexpr auto noneType = InpType::NONE;
			constexpr auto downType = InpType::KEYDOWN;
			constexpr auto repeatType = InpType::KEYREPEAT;
			constexpr auto upType = InpType::KEYUP;

			const auto lastAction = m_currentMapping->ControllerButtonState.LastAction;

			// If this cbtam is being asked to do down/repeat/up.
			const bool AskUp = stroke.KeyUp && stroke.VirtualKey == m_currentMapping->ControllerButton.VK;
			const bool AskDown = stroke.KeyDown && stroke.VirtualKey == m_currentMapping->ControllerButton.VK;
			const bool AskRepeat = stroke.KeyRepeat && stroke.VirtualKey == m_currentMapping->ControllerButton.VK;

			// If the current state will allow it to actually do down/repeat/up.
			const bool DoDown = ((lastAction == upType) || (lastAction == noneType)) && AskDown;
			const bool DoRepeat = (lastAction == repeatType || lastAction == downType) && AskRepeat && m_currentMapping->KeymapData.UsesRepeat;
			const bool DoUp = (lastAction == downType || lastAction == repeatType) && AskUp;

			// Then do down/repeat/up.
			if (DoDown)
			{
				auto overtakingAction = IsOvertaking(*m_currentMapping);
				//const InpType inputType = DoDown ? downType : repeatType;
				std::visit([&](auto& t) { t(downType); }, overtakingAction);
			}
			else if (DoRepeat)
			{
				//Repeat'd key cannot be overtaking, only a newly key-down'd key.
				NormalKeyDown_t t{ m_currentMapping };
				t(InpType::KEYREPEAT);
			}
			else if (DoUp)
			{
				KeyUp_t k(m_currentMapping);
				k(InpType::KEYUP);
			}
		}

		/**
		 * \brief Check to see if an exclusivity grouping has been pressed already
		 * \param detail Newest element being set to keydown state
		 * \return one of three states, to be std::visit'd for what to do in that state
		 */
		[[nodiscard]]
		auto IsOvertaking(const ControllerButtonToActionMap& detail) noexcept
		-> std::variant<OvertakingKeyDown_t, NoUpdate_t, NormalKeyDown_t>
		{
			using std::ranges::all_of, std::ranges::find, std::ranges::find_if, std::ranges::begin, std::ranges::end;
			// Get copy of range to pointers to all mappings in existence.
			const auto mapBuffer = detail.GetThisBuffer();
			// Get copy of pointers to all mappings in the same exclusivity grouping as this mapping.
			const auto groupedBuffer = std::ranges::views::transform(mapBuffer, [exGroup = m_currentMapping->KeymapData.ExclusivityGrouping](const auto& elem)
				{
					return elem->KeymapData.ExclusivityGrouping == exGroup;
				});
			const auto groupedNoUpdateBuffer = std::ranges::views::transform(mapBuffer, [exGroup = m_currentMapping->KeymapData.ExclusivityNoUpdateGrouping](const auto& elem)
				{
					return elem->KeymapData.ExclusivityNoUpdateGrouping == exGroup;
				});

			// If one or the other has members, assert that they don't BOTH have members as this is obviously
			// a problem.
			if(!groupedBuffer.empty() || !groupedNoUpdateBuffer.empty())
				assert(groupedBuffer.empty() ^ groupedNoUpdateBuffer.empty());

			const auto groupedResult = GetGroupedOvertaken(detail, groupedBuffer);
			const auto groupedNoUpdateResult = GetGroupedOvertaken(detail, groupedNoUpdateBuffer);
			if (groupedResult)
				return OvertakingKeyDown_t{ groupedResult.value(), m_currentMapping };
			if (groupedNoUpdateResult)
				return NoUpdate_t{};
			return NormalKeyDown_t{m_currentMapping};
		}
	private:
		/**
		 * \brief Checks the exclusivity group members for a key being overtaken by the newly pressed key.
		 * \param detail button newly pressed
		 * \param groupedBuffer buffer holding the state of all keys at present within the grouping
		 * \return optional, pointer to cbtam being overtaken
		 */
		auto GetGroupedOvertaken(
			const ControllerButtonToActionMap& detail,
			const auto& groupedBuffer) -> std::optional<ControllerButtonToActionMap*>
		{
			using std::ranges::find_if, std::ranges::end;
			// If exclusivity with update grouping has some other members...
			if (!groupedBuffer.empty())
			{
				auto IsGroupedBtnPressedFn = [&](const ControllerButtonToActionMap* groupedButtonElem)
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

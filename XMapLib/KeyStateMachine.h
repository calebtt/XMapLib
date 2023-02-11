#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include <iostream>
#include <chrono>
#include <optional>
#include <type_traits>
#include <concepts>

namespace sds
{
	// Concept for keymap info type, provides things like GetLastAction() and IsExclusivityBlocked()
	template<typename T>
	concept KMInfo_c = requires (T & t)
	{
		// Information
		{ t.GetMapBuffer() } -> std::ranges::range;
		{ t.GetLastAction() } -> ControllerButtonStateData::ActionType;
		{ t.GetButtonVK() } -> int;
		{ t.IsExclusivityBlocked() } -> std::convertible_to<bool>;
		// Data members
		{ t.ControllerButtonState };
		{ t.KeymapData };
	};
	// Concept for keymap actions type, provides things like DoDown() and DoUp()
	template<typename T>
	concept KMActions_c = requires (T & t)
	{
		// Actions
		{ t.DoDown() };
		{ t.DoRepeat() };
		{ t.DoUp() };
	};

	/**
	 * \brief Instances are attached to CBTAM mappings, It operates a state machine for the case of
	 * controller button to keyboard button mappings. Just feed it ControllerStateWrapper structs.
	 */
	//template<IsMapInfoSource MappingInfoSource>
	template<KMInfo_c MapInfoSource_t, KMActions_c MapActionsSource_t>
	struct KeyStateMachine
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
		MapInfoSource_t* mpInfo{};
		MapActionsSource_t* mpActions{};
		KeyboardSettingsPack m_ksp;
	public:
		explicit KeyStateMachine(
			MapInfoSource_t&& mapInfo,
			MapActionsSource_t&& mapActions,
			const KeyboardSettingsPack& ksp = {})
			: mpInfo(std::move(mapInfo)), mpActions(std::move(mapActions)), m_ksp(ksp)
		{

		}
	public:
		/**
		 * \brief Entry point for key-down state machine logic. Manages explicitly the requested action.
		 * Then performs the KeyRepeat loop and the KeyUpdate loop (resets keys for use again).
		 * \param stroke info about controller key-press event.
		 */
		void DoDown(const ControllerStateWrapper& stroke) noexcept
		{
			constexpr auto noneType = InpType::NONE;
			constexpr auto upType = InpType::KEYUP;

			const auto lastAction = mpInfo.GetLastAction();

			// If this cbtam is being asked to do down/repeat/up.
			const bool AskDown = stroke.KeyDown && stroke.VirtualKey == m_currentMapping.ControllerButton.VK;

			// If the current state will allow it to actually do down/repeat/up.
			const bool DoDown = ((lastAction == upType) || (lastAction == noneType)) && AskDown;

			// Then do down/repeat/up.
			if (DoDown)
			{
				mpActions.DoDown();
			}
		}

		/**
		 * \brief For key-up state machine logic. Manages explicitly the requested action.
		 * Then performs the KeyRepeat loop and the KeyUpdate loop (resets keys for use again).
		 * \param stroke info about controller key-press event.
		 */
		void DoUp(const ControllerStateWrapper& stroke) noexcept
		{
			constexpr auto downType = InpType::KEYDOWN;
			constexpr auto repeatType = InpType::KEYREPEAT;

			const auto lastAction = mpInfo.GetLastAction();

			// If this cbtam is being asked to do down/repeat/up.
			const bool AskUp = stroke.KeyUp && stroke.VirtualKey == mpInfo.GetButtonVK();

			// If the current state will allow it to actually do down/repeat/up.
			const bool DoUp = (lastAction == downType || lastAction == repeatType) && AskUp;

			// Then do down/repeat/up.
			if (DoUp)
			{
				mpInfo.DoUp();
			}
		}

		/**
		 * \brief For key-repeat state machine logic. Manages explicitly the requested action.
		 *  Also performs the KeyRepeat loop and the KeyUpdate loop (resets keys for use again).
		 * \param stroke info about controller key-press event.
		 */
		void DoRepeat(const ControllerStateWrapper& stroke) noexcept
		{
			constexpr auto downType = InpType::KEYDOWN;
			constexpr auto repeatType = InpType::KEYREPEAT;

			const auto lastAction = mpInfo.GetLastAction();

			// If this cbtam is being asked to do down/repeat/up.
			const bool AskRepeat = stroke.KeyRepeat && stroke.VirtualKey == mpInfo.GetButtonVK();

			// If the current state will allow it to actually do down/repeat/up.
			const bool DoRepeat = (lastAction == repeatType || lastAction == downType) && AskRepeat && m_currentMapping.KeymapData.UsesRepeat;

			// Then do down/repeat/up.
			if (DoRepeat)
			{
				//Repeat'd key cannot be overtaking, only a newly key-down'd key.
				mpActions.DoRepeat();
			}
		}
	private:

	};
}

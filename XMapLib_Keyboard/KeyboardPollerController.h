#pragma once
#include "LibIncludes.h"
#include "CustomTypes.h"

#include "ControllerStateWrapper.h"
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
		{ t.GetUpdatedState() } -> std::convertible_to<ControllerStateWrapper>;
	};

	[[nodiscard]] constexpr bool IsCodeMaskedForAnyTrigger(const detail::VirtualKey_t state) noexcept
	{
		constexpr auto rightValue{ VK_PAD_RTRIGGER };
		constexpr auto leftValue{ VK_PAD_LTRIGGER };
		return state & leftValue || state & rightValue;
	}
	[[nodiscard]] constexpr bool IsCodeMaskedForLeftTrigger(const detail::VirtualKey_t virtualKeycode) noexcept
	{
		constexpr auto leftValue{ VK_PAD_LTRIGGER };
		return virtualKeycode & leftValue;
	}
	[[nodiscard]] constexpr bool IsCodeMaskedForRightTrigger(const detail::VirtualKey_t virtualKeycode) noexcept
	{
		constexpr auto rightValue{ VK_PAD_RTRIGGER };
		return virtualKeycode & rightValue;
	}
	[[nodiscard]] constexpr bool IsLeftTriggerBeyondThreshold(const detail::TriggerValue_t triggerValue, const detail::TriggerValue_t triggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD) noexcept
	{
		return triggerValue > triggerThreshold;
	}
	[[nodiscard]] constexpr bool IsRightTriggerBeyondThreshold(const detail::TriggerValue_t triggerValue, const detail::TriggerValue_t triggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD) noexcept
	{
		return triggerValue > triggerThreshold;
	}

	// If the mapping state is at 'initial', call this to update it.
	inline
	auto UpdateButtonStateForInitial(KeyboardSettings& settings, CBActionMap& singleButton, const XINPUT_STATE& controllerState) -> detail::StaticVector_t<TranslationResult>
	{
		detail::StaticVector_t<TranslationResult> translatedUpdates;
		const auto virtualCodeMask = controllerState.Gamepad.wButtons;

		// Triggers...
		const bool isLeftTrigger = IsCodeMaskedForLeftTrigger(virtualCodeMask);
		const bool isRightTrigger = IsCodeMaskedForRightTrigger(virtualCodeMask);
		if(isLeftTrigger || isRightTrigger)
		{
			const std::function thresholdTester = isLeftTrigger ? IsLeftTriggerBeyondThreshold : IsRightTriggerBeyondThreshold;
			const auto activationValue = isLeftTrigger ? settings.LeftTriggerThreshold : settings.RightTriggerThreshold;
			if (thresholdTester(controllerState.Gamepad.bLeftTrigger, activationValue))
			{
				translatedUpdates.emplace_back(GetInitialKeyDownTranslationResult(singleButton));
			}
		}

		// Buttons...
		const bool isButtonDown = virtualCodeMask & singleButton.ButtonVirtualKeycode;
		if(isButtonDown)
		{
			translatedUpdates.emplace_back(GetInitialKeyDownTranslationResult(singleButton));
		}

		// Thumbstick directions...
		// TODO look at old code, take the part that recognizes thumbsticks being beyond dz.

		// TODO also, it may be useful still to extract the XINPUT_STATE bitmask buttons into a vector of wrappers. Could use them in the ControllerStateWrapper buffer class and not depend on the OS API type.

		return translatedUpdates;
	}

	//inline
	//auto SetButtonUpdatesFromLegacyPlatformApi(ControllerStateWrapper<>& stateBuffer, const XINPUT_STATE& controllerState)
	//{
	//	for(auto& button : stateBuffer.Buttons)
	//	{

	//		if(button.ButtonVirtualKeycode & controllerState.Gamepad.wButtons)
	//		{
	//		}
	//	}
	//}
	//[[nodiscard]]
	//inline
	//auto GetWrappedStateFromLegacyPlatformApi(const XINPUT_STATE& controllerState) noexcept -> ControllerStateWrapper
	//{
	//	PlatformControllerApiWrapper tempWrapper;
	//	for(auto& buttonStatus : )
	//}

	class KeyboardPollerControllerLegacy
	{
		enum class KeyStates : int
		{
			INITIAL,
			DOWN,
			REPEAT
		};
	private:
		int m_playerId;
		ControllerStateWrapper<> m_controllerStates;
	public:

		KeyboardPollerControllerLegacy() = delete;
		explicit KeyboardPollerControllerLegacy(const int pid) : m_playerId(pid) { }
	public:
		/**
		 * \brief Returns an updated ControllerStateWrapper containing information gathered about a controller keypress.
		 */
		[[nodiscard]]
		auto operator()() noexcept -> ControllerStateWrapper
		{
			return GetUpdatedState();
		}

		/**
		 * \brief Returns an updated ControllerStateWrapper containing information gathered about a controller keypress.
		 */
		[[nodiscard]]
		auto GetUpdatedState() noexcept -> ControllerStateWrapper
		{
			const auto newResult = GetLegacyApiStateUpdate(m_playerId);
			if (newResult.has_value())
			{
				if (IsVirtualKeyForATrigger(*newResult))
				{
					// If it's for a trigger then we call the old API to get a state for the triggers.
					const auto oldErr = XInputGetState(m_playerId, &m_oldState);
					const auto newTriggerDown = GetVkForDownTrigger();
					const auto newTriggerUp = GetVkForUpTrigger();
					const auto newTriggerRepeat = GetVkForRepeatTrigger();
					if (newTriggerDown)
					{
						return ControllerStateWrapper{
							.VirtualKey = static_cast<unsigned short>(newTriggerDown.value()),
							.KeyDown = true,
							.KeyUp = false,
							.KeyRepeat = false
						};
					}
					if (newTriggerUp)
					{
						return ControllerStateWrapper{
							.VirtualKey = static_cast<unsigned short>(newTriggerUp.value()),
							.KeyDown = false,
							.KeyUp = true,
							.KeyRepeat = false
						};
					}
					if (newTriggerRepeat)
					{
						return ControllerStateWrapper{
							.VirtualKey = static_cast<unsigned short>(newTriggerRepeat.value()),
							.KeyDown = false,
							.KeyUp = false,
							.KeyRepeat = true
						};
					}
				}
				return *newResult;
			}

			return {};
		}
	};


}
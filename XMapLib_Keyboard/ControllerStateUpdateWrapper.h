#pragma once
#include "KeyboardLibIncludes.h"
#include "ControllerButtonToActionMap.h"

namespace sds
{
	/**
	 * \brief The wButtons member of the OS API struct is ONLY for the buttons, triggers are not set there on a key-down.
	 */
	[[nodiscard]] constexpr bool IsLeftTriggerBeyondThreshold(const detail::TriggerValue_t triggerValue, const detail::TriggerValue_t triggerThreshold) noexcept
	{
		return triggerValue > triggerThreshold;
	}

	[[nodiscard]] constexpr bool IsRightTriggerBeyondThreshold(const detail::TriggerValue_t triggerValue, const detail::TriggerValue_t triggerThreshold) noexcept
	{
		return triggerValue > triggerThreshold;
	}

	/**
	 * \brief Concept for a controller settings object that provides a controller button virtual keycode array with
	 *	the right properties.
	 */
	template<typename Settings_t>
	concept ControllerSettings_c = requires(Settings_t &settings)
	{
		// has compile-time accessible value for the number of controller buttons, and is convertible to size_t
		{ std::convertible_to<decltype(std::size(Settings_t::ButtonCodeArray)), std::size_t> };
		// number of controller buttons is also greater than 0
		{ std::size(Settings_t::ButtonCodeArray) > 0 };
		{ std::convertible_to<decltype(settings.LeftTriggerThreshold), int> };
		{ std::convertible_to<decltype(settings.RightTriggerThreshold), int> };
		{ std::convertible_to<decltype(settings.LeftStickDeadzone), int> };
		{ std::convertible_to<decltype(settings.RightStickDeadzone), int> };
	};

	// Concept for a state update wrapper that probably depends on some platform specific type or behavior.
	template<typename Wrapper_t>
	concept ControllerStateOperations_c = requires(Wrapper_t & wrapperInstance)
	{
		// Value dependent button test function
		{ wrapperInstance.IsButtonDown(123) } -> std::convertible_to<bool>;
		// Triggers, left and right
		{ wrapperInstance.IsLeftTriggerDown() } -> std::convertible_to<bool>;
		{ wrapperInstance.IsRightTriggerDown() } -> std::convertible_to<bool>;
		// Left thumbstick direction values
		{ wrapperInstance.IsLeftThumbstickLeftDown() } -> std::convertible_to<bool>;
		{ wrapperInstance.IsLeftThumbstickRightDown() } -> std::convertible_to<bool>;
		{ wrapperInstance.IsLeftThumbstickUpDown() } -> std::convertible_to<bool>;
		{ wrapperInstance.IsLeftThumbstickDownDown() } -> std::convertible_to<bool>;
		// Right thumbstick direction values
		{ wrapperInstance.IsRightThumbstickLeftDown() } -> std::convertible_to<bool>;
		{ wrapperInstance.IsRightThumbstickRightDown() } -> std::convertible_to<bool>;
		{ wrapperInstance.IsRightThumbstickUpDown() } -> std::convertible_to<bool>;
		{ wrapperInstance.IsRightThumbstickDownDown() } -> std::convertible_to<bool>;
		{ wrapperInstance.GetDownVirtualKeycodesRange() } -> std::convertible_to<detail::SmallVector_t<int>>;
	};

	/**
	 * \brief Current controller button state info wrapper. Construct an instance to get some info about the state.
	 */
	template<ControllerSettings_c ConfigSettings_t = KeyboardSettings>
	class ControllerStateUpdateWrapper
	{
		static_assert(ControllerSettings_c<ConfigSettings_t>);
		XINPUT_STATE m_controllerStates;
		ConfigSettings_t m_settings;
	public:
		constexpr
		ControllerStateUpdateWrapper(const XINPUT_STATE newStates) noexcept
			: m_controllerStates(newStates)
		{ }

		[[nodiscard]] constexpr bool IsButtonDown(const detail::VirtualKey_t buttonId) const noexcept
		{
			return m_controllerStates.Gamepad.wButtons & buttonId;
		}

		[[nodiscard]] constexpr bool IsLeftTriggerDown() const noexcept { return IsLeftTriggerBeyondThreshold(m_controllerStates.Gamepad.bLeftTrigger, m_settings.LeftTriggerThreshold); }
		[[nodiscard]] constexpr bool IsRightTriggerDown() const noexcept { return IsRightTriggerBeyondThreshold(m_controllerStates.Gamepad.bRightTrigger, m_settings.RightTriggerThreshold); }

		[[nodiscard]] constexpr bool IsLeftThumbstickLeftDown() const noexcept { return m_controllerStates.Gamepad.sThumbLX < -m_settings.LeftStickDeadzone; }
		[[nodiscard]] constexpr bool IsLeftThumbstickRightDown() const noexcept { return m_controllerStates.Gamepad.sThumbLX > m_settings.LeftStickDeadzone; }
		[[nodiscard]] constexpr bool IsLeftThumbstickUpDown() const noexcept { return m_controllerStates.Gamepad.sThumbLY > m_settings.LeftStickDeadzone; }
		[[nodiscard]] constexpr bool IsLeftThumbstickDownDown() const noexcept { return m_controllerStates.Gamepad.sThumbLY < -m_settings.LeftStickDeadzone; }

		[[nodiscard]] constexpr bool IsRightThumbstickLeftDown() const noexcept { return m_controllerStates.Gamepad.sThumbRX < -m_settings.RightStickDeadzone; }
		[[nodiscard]] constexpr bool IsRightThumbstickRightDown() const noexcept { return m_controllerStates.Gamepad.sThumbRX > m_settings.RightStickDeadzone; }
		[[nodiscard]] constexpr bool IsRightThumbstickUpDown() const noexcept { return m_controllerStates.Gamepad.sThumbRY > m_settings.RightStickDeadzone; }
		[[nodiscard]] constexpr bool IsRightThumbstickDownDown() const noexcept { return m_controllerStates.Gamepad.sThumbRY < -m_settings.RightStickDeadzone; }

		[[nodiscard]] constexpr auto GetDownVirtualKeycodesRange() const noexcept -> detail::SmallVector_t<int>
		{
			detail::SmallVector_t<int> allKeys{};
			for(const auto elem: m_settings.ButtonCodeArray)
			{
				if(elem & m_controllerStates.Gamepad.wButtons)
				{
					allKeys.emplace_back(elem);
				}
			}
			if (IsLeftTriggerDown())
				allKeys.emplace_back(m_settings.LeftTriggerVk);
			if (IsRightTriggerDown())
				allKeys.emplace_back(m_settings.RightTriggerVk);

			if (IsLeftThumbstickLeftDown())
				allKeys.emplace_back(m_settings.LeftThumbstickLeft);
			if (IsLeftThumbstickRightDown())
				allKeys.emplace_back(m_settings.LeftThumbstickRight);
			if (IsLeftThumbstickUpDown())
				allKeys.emplace_back(m_settings.LeftThumbstickUp);
			if (IsLeftThumbstickDownDown())
				allKeys.emplace_back(m_settings.LeftThumbstickDown);

			if (IsRightThumbstickLeftDown())
				allKeys.emplace_back(m_settings.RightThumbstickLeft);
			if (IsRightThumbstickRightDown())
				allKeys.emplace_back(m_settings.RightThumbstickRight);
			if (IsRightThumbstickUpDown())
				allKeys.emplace_back(m_settings.RightThumbstickUp);
			if (IsRightThumbstickDownDown())
				allKeys.emplace_back(m_settings.RightThumbstickDown);
			return allKeys;
		}

	};
	static_assert(ControllerStateOperations_c<ControllerStateUpdateWrapper<KeyboardSettings>>);

}
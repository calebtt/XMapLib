#pragma once
#include "LibIncludes.h"
#include "ControllerButtonToActionMap.h"

namespace sds
{
	/**
	 * \brief Concept for a controller settings object that provides a controller button virtual keycode array with
	 *	the right properties.
	 */
	template<typename Settings_t>
	concept HasControllerSettings = requires(Settings_t &settings)
	{
		// has compile-time accessible value for the number of controller buttons, and is convertible to size_t
		{ std::convertible_to<decltype(std::size(Settings_t::ButtonCodeArray)), std::size_t> };
		// number of controller buttons is also greater than 0
		{ std::size(Settings_t::ButtonCodeArray) > 0 };
	};

	/**
	 * \brief Current controller button state buffer. An element and state object for each button.
	 */
	template<HasControllerSettings Settings_t = KeyboardSettings>
	struct ControllerStateWrapper
	{
		std::array<CBActionMap, std::size(Settings_t::ButtonCodeArray)> Buttons;
		static_assert(std::size(Buttons) == std::size(Settings_t::ButtonCodeArray));
	public:
		constexpr
		ControllerStateWrapper() noexcept
		{
			for(std::size_t i{}; i < std::size(Buttons); ++i)
			{
				Buttons[i].ButtonVirtualKeycode = Settings_t::ButtonCodeArray[i];
			}
		}

	};

}
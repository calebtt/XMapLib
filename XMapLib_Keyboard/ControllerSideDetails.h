#pragma once
#include "LibIncludes.h"
#include <any>
#include <ostream>
#include "KeyboardSettingsPack.h"

// Most of these structs are used by ControllerButtonToActionMap
namespace sds
{
	/**
	 * \brief The specific data used to describe the controller button in the mapping. This type is probably relatively concrete within the mapping logic.
	 */
	struct ControllerButtonData
	{
		// VK of controller button
		int VK{ 0 };

		friend bool operator==(const ControllerButtonData& lhs, const ControllerButtonData& rhs)
		{
			return lhs.VK == rhs.VK;
		}

		friend bool operator!=(const ControllerButtonData& lhs, const ControllerButtonData& rhs)
		{
			return !(lhs == rhs);
		}
	};

	/**
	 * \brief The specific data used to describe the mapped-to (kbd/mouse) key in the mapping.
	 */
	struct KeyboardButtonData
	{
		// Presumably, the VK of mapped-to input (key or mouse button)
		// but if some other data is stored here, that is fine too.
		std::any VK;
	};

	/**
	 * \brief The extra information regarding the controller button to keyboard key mapping.
	 */
	struct ControllerToKeyMapData
	{
		using GroupingProperty_t = int;
		using Cms_t = std::chrono::milliseconds;
		// Uses the key-repeat behavior when held down
		bool UsesRepeat{ true };
		// Delay before activating the key-repeat behavior.
		Cms_t DelayBeforeRepeatActivation{ Cms_t{500} };
		// Delay after or in-between key repeat events.
		Cms_t DelayAfterRepeatActivation{ Cms_t{250} };
		// Grouping for exclusivity with overtaking, if a new key is pressed while
		// a key in the grouping is already pressed, the new press overtakes the old.
		GroupingProperty_t ExclusivityGrouping{};
		// Grouping for exclusivity with no overtaking, if a new key is pressed while
		// a key in the grouping is already pressed, nothing happens.
		GroupingProperty_t ExclusivityNoOvertakingGrouping{};
	};

	/**
	 * \brief Specific info regarding the state machine which is used to track the events being fired, based on what the controller button reports.
	 */
	struct ControllerButtonStateData
	{
		using Delay_t = DelayManagement::DelayManager<std::chrono::microseconds>;
		enum class ActionType : int
		{
			NONE = 0,
			KEYDOWN = XINPUT_KEYSTROKE_KEYDOWN,
			KEYREPEAT = XINPUT_KEYSTROKE_REPEAT,
			KEYUP = XINPUT_KEYSTROKE_KEYUP
		};

		// state machine info for controller btn
		ActionType LastAction{ ActionType::NONE };
		// last sent time, normally used for key repeat
		DelayManagement::DelayManager<std::chrono::microseconds> LastSentTime{ KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT };

		friend std::ostream& operator<<(std::ostream& os, const ControllerButtonStateData& obj)
		{
			constexpr bool isInt = std::is_same_v<std::underlying_type<ActionType>, int>;
			if constexpr(isInt)
			{
				return os
					<< "LastAction: " << static_cast<int>(obj.LastAction)
					<< " LastSentTime: " << obj.LastSentTime;
			}
			return os << "ERROR";
		}
	};
}

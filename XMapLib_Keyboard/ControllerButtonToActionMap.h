#pragma once
#include "LibIncludes.h"
#include <syncstream>
#include <typeindex>
#include "KeyboardSettingsPack.h"
#include "KeyStateMachineInformational.h"

namespace sds
{
	/**
	 * \brief Wrapper for key mapping state enum, the least I can do is make sure state modifications occur through a managing class,
	 * and that there exists only one 'current' state, and that it can only be a finite set of possibilities.
	 * Also contains last sent time and a keyboard settings pack.
	 */
	class MappingStateManager
	{
		enum class ActionState : int
		{
			INIT, // State indicating ready for new cycle
			KEYDOWN,
			KEYREPEAT,
			KEYUP,
		};
		ActionState m_currentValue{ ActionState::INIT };
		KeyboardSettings m_keyDefaults{};
	public:
		DelayManagement::DelayManager LastSentTime{ std::chrono::microseconds{m_keyDefaults.MICROSECONDS_DELAY_KEYREPEAT} };
	public:
		[[nodiscard]] constexpr bool IsRepeating() const noexcept { return m_currentValue == ActionState::KEYREPEAT; }
		[[nodiscard]] constexpr bool IsDown() const noexcept { return m_currentValue == ActionState::KEYDOWN; }
		[[nodiscard]] constexpr bool IsUp() const noexcept { return m_currentValue == ActionState::KEYUP; }
		[[nodiscard]] constexpr bool IsInitialState() const noexcept { return m_currentValue == ActionState::INIT; }
		constexpr auto SetDown() noexcept { m_currentValue = ActionState::KEYDOWN; }
		constexpr auto SetUp() noexcept { m_currentValue = ActionState::KEYUP; }
		constexpr auto SetRepeat() noexcept { m_currentValue = ActionState::KEYREPEAT; }
		constexpr auto SetInitial() noexcept { m_currentValue = ActionState::INIT; }
	};

	static_assert(std::is_copy_constructible_v<MappingStateManager>);
	static_assert(std::is_copy_assignable_v<MappingStateManager>);

	/**
	 * \brief	Controller button to action mapping. This is how a mapping of a controller button to an action is described.
	 */
	struct CBActionMap
	{
		/**
		 * \brief	Controller button Virtual Keycode. Can be platform dependent or custom mapping, depends on input poller behavior.
		 */
		int Vk{};
		/**
		 * \brief	If 'true', upon the button being held down, will translate to the key-repeat function repeatedly using a delay in between repeats.
		 */
		bool UsesRepeatBehavior{ true };
		/**
		 * \brief	If 'true', upon the button being held down, will send a single repeat, will not continue translating to repeat after the single repeat.
		 * \remarks Note that UsesRepeatBehavior is expected to be set to 'false' for this to have a meaningful impact.
		 */
		bool SendsFirstRepeatOnly{ false };
		/**
		 * \brief The exclusivity grouping member is intended to allow the user to add different groups of mappings
		 * that require another mapping from the same group to be "overtaken" or key-up sent before the "overtaking" new mapping
		 * can perform the key-down.
		 * \remarks optional, if not in use set to default constructed value or '{}'
		 */
		detail::OptGrp_t ExclusivityGrouping;
		detail::Fn_t OnDown; // Key-down
		detail::Fn_t OnUp; // Key-up
		detail::Fn_t OnRepeat; // Key-repeat
		detail::Fn_t OnReset; // Reset after key-up prior to another key-down
		detail::Delay_t PriorToRepeatDelay; // TODO
		detail::OptDelay_t CustomRepeatDelay; // optional custom delay between key-repeats
		MappingStateManager LastAction; // Last action performed, with get/set methods.
	};
}

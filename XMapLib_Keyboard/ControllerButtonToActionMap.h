#pragma once
#include "KeyboardLibIncludes.h"
#include "KeyboardSettingsPack.h"

namespace sds
{
	enum class ActionState : int
	{
		INIT, // State indicating ready for new cycle
		KEYDOWN,
		KEYREPEAT,
		KEYUP,
	};

	/**
	 * \brief Wrapper for key mapping state enum, the least I can do is make sure state modifications occur through a managing class,
	 * and that there exists only one 'current' state, and that it can only be a finite set of possibilities.
	 * Also contains last sent time (for key-repeat), delay before first key-repeat timer, and a keyboard settings pack.
	 */
	class MappingStateManager
	{
		ActionState m_currentValue{ ActionState::INIT };
		KeyboardSettings m_keyDefaults{};
	public:
		/**
		 * \brief	This delay is mostly used for in-between key-repeats, but could also be in between other state transitions.
		 */
		DelayManagement::DelayManager LastSentTime{ m_keyDefaults.KeyRepeatDelay };
		/**
		 * \brief	This is the delay before the first repeat is sent whilst holding the button down.
		 */
		DelayManagement::DelayManager DelayBeforeFirstRepeat{ LastSentTime.GetTimerPeriod() };
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
		detail::VirtualKey_t ButtonVirtualKeycode{};
		/**
		 * \brief	If 'true', upon the button being held down, will translate to the key-repeat function activating repeatedly
		 *	using a delay in between repeats.
		 */
		bool UsesInfiniteRepeat{ true };
		/**
		 * \brief	If 'true', upon the button being held down, will send a single repeat, will not continue translating to repeat after the single repeat.
		 * \remarks Note that UsesInfiniteRepeat is expected to be set to 'false' for this to have a meaningful impact.
		 */
		bool SendsFirstRepeatOnly{ false };
		/**
		 * \brief	The exclusivity grouping member is intended to allow the user to add different groups of mappings
		 *	that require another mapping from the same group to be "overtaken" or key-up sent before the "overtaking" new mapping
		 *	can perform the key-down.
		 * \remarks		optional, if not in use set to default constructed value or '{}'
		 */
		detail::OptGrp_t ExclusivityGrouping;
		detail::Fn_t OnDown; // Key-down
		detail::Fn_t OnUp; // Key-up
		detail::Fn_t OnRepeat; // Key-repeat
		detail::Fn_t OnReset; // Reset after key-up prior to another key-down
		detail::OptNanosDelay_t DelayBeforeFirstRepeat; // optional custom delay before first key-repeat
		detail::OptNanosDelay_t DelayForRepeats; // optional custom delay between key-repeats
		MappingStateManager LastAction; // Last action performed, with get/set methods.
	};
	static_assert(std::is_copy_constructible_v<CBActionMap>);
	static_assert(std::is_copy_assignable_v<CBActionMap>);

}

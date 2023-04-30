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
	 * \brief The exclusivity grouping member is intended to allow the user to add different groups of mappings
	 * that require another mapping from the same group to be "overtaken" or key-up sent before the "overtaking" new mapping
	 * can perform the key-down.
	 */
	struct CBActionMap
	{
		int Vk{};
		bool UsesRepeat{ true };
		detail::OptGrp_t ExclusivityGrouping;
		detail::Fn_t OnDown;
		detail::Fn_t OnUp;
		detail::Fn_t OnRepeat;
		detail::Fn_t OnReset;
		detail::OptDelay_t CustomRepeatDelay; // optional custom delay between key-repeats
		MappingStateManager LastAction; // Last action performed, with get/set methods.
	};
}

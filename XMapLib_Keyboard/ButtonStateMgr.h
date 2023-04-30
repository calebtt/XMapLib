#pragma once
#include "LibIncludes.h"

namespace sds
{
	class ButtonStateMgr
	{
	public:
		enum class ActionState : int
		{
			INIT, // State indicating ready for new cycle
			KEYDOWN,
			KEYREPEAT,
			KEYUP,
		};
	private:
		ActionState m_currentValue{ ActionState::INIT };
	public:
		ButtonStateMgr() = default;
		ButtonStateMgr(const ActionState as) : m_currentValue(as) { }
		[[nodiscard]] constexpr bool IsRepeating() const noexcept { return m_currentValue == ActionState::KEYREPEAT; }
		[[nodiscard]] constexpr bool IsDown() const noexcept { return m_currentValue == ActionState::KEYDOWN; }
		[[nodiscard]] constexpr bool IsUp() const noexcept { return m_currentValue == ActionState::KEYUP; }
		[[nodiscard]] constexpr bool IsInitialState() const noexcept { return m_currentValue == ActionState::INIT; }
		constexpr auto SetDown() noexcept { m_currentValue = ActionState::KEYDOWN; }
		constexpr auto SetUp() noexcept { m_currentValue = ActionState::KEYUP; }
		constexpr auto SetRepeat() noexcept { m_currentValue = ActionState::KEYREPEAT; }
		constexpr auto SetInitial() noexcept { m_currentValue = ActionState::INIT; }
	};

	static_assert(std::is_copy_constructible_v<ButtonStateMgr>);
	static_assert(std::is_copy_assignable_v<ButtonStateMgr>);

	// Free functions intended to provide a ButtonStateMgr instance with a state set.
	[[nodiscard]]
	inline
	auto ButtonStateMgrUp() -> ButtonStateMgr { return ButtonStateMgr::ActionState::KEYUP; }
	[[nodiscard]]
	inline
	auto ButtonStateMgrDown() -> ButtonStateMgr { return ButtonStateMgr::ActionState::KEYDOWN; }
	[[nodiscard]]
	inline
	auto ButtonStateMgrRepeat() -> ButtonStateMgr { return ButtonStateMgr::ActionState::KEYREPEAT; }
	[[nodiscard]]
	inline
	auto ButtonStateMgrInitial() -> ButtonStateMgr { return ButtonStateMgr::ActionState::INIT; }
}
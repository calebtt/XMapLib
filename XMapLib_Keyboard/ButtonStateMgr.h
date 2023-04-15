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
		ButtonStateMgr(ActionState as) : m_currentValue(as) { }
		[[nodiscard]] constexpr bool IsRepeating() const noexcept { return m_currentValue == ActionState::KEYREPEAT; }
		[[nodiscard]] constexpr bool IsDown() const noexcept { return m_currentValue == ActionState::KEYDOWN; }
		[[nodiscard]] constexpr bool IsUp() const noexcept { return m_currentValue == ActionState::KEYUP; }
		[[nodiscard]] constexpr bool IsInitialState() const noexcept { return m_currentValue == ActionState::INIT; }
		auto SetDown() noexcept { m_currentValue = ActionState::KEYDOWN; }
		auto SetUp() noexcept { m_currentValue = ActionState::KEYUP; }
		auto SetRepeat() noexcept { m_currentValue = ActionState::KEYREPEAT; }
		auto SetInitial() noexcept { m_currentValue = ActionState::INIT; }
	};
}
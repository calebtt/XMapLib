#pragma once
#include "LibIncludes.h"

namespace sds
{
	class PriorityMgr
	{
	public:
		enum class PriorityState : int
		{
			INVALID, // Invalid for use!
			REPEAT, // key repeating
			UPDATE,  // key updating
			EX_OVERTAKING, // keys being overtaken
			NEXT_STATE, // simple state transition to next state
		};
	private:
		PriorityState m_currentValue{ PriorityState::INVALID };
	public:
		PriorityMgr() = default;
		PriorityMgr(PriorityState p) : m_currentValue(p) { }
		[[nodiscard]] constexpr bool IsRepeat() const noexcept { return m_currentValue == PriorityState::REPEAT; }
		[[nodiscard]] constexpr bool IsNextState() const noexcept { return m_currentValue == PriorityState::NEXT_STATE; }
		[[nodiscard]] constexpr bool IsUpdate() const noexcept { return m_currentValue == PriorityState::UPDATE; }
		[[nodiscard]] constexpr bool IsOvertaking() const noexcept { return m_currentValue == PriorityState::EX_OVERTAKING; }
		[[nodiscard]] constexpr bool IsInvalidState() const noexcept { return m_currentValue == PriorityState::INVALID; }
		auto SetRepeat() noexcept { m_currentValue = PriorityState::REPEAT; }
		auto SetNextState() noexcept { m_currentValue = PriorityState::NEXT_STATE; }
		auto SetUpdate() noexcept { m_currentValue = PriorityState::UPDATE; }
		auto SetOvertaking() noexcept { m_currentValue = PriorityState::EX_OVERTAKING; }
		//auto SetOvertaking() && noexcept { m_currentValue = PriorityState::EX_OVERTAKING; return m_currentValue; }
		auto SetInvalid() noexcept { m_currentValue = PriorityState::INVALID; }
	};
}
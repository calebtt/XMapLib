#pragma once
#include "LibIncludes.h"
#include <syncstream>
#include <typeindex>
#include "ControllerSideDetails.h"
#include "KeyStateMachineInformational.h"

namespace sds
{
	/**
	 * \brief Takes a range of pause-able tasks and returns a single std::function that calls them in succession.
	 * \remarks The range of functions is copied into the lambda.
	 * \param taskList an iterable range of std::function tasks.
	 * \return returns the newly created singular std::function that calls all of the tasks in succession.
	 */
	template<class FnRange_t>
	auto make_async_runnable_package(const FnRange_t& taskList)	-> std::function<void()>
	{
		return std::function<void()>
		{[taskList]()
			{
				for (const auto& elem : taskList)
				{
					elem();
				}
			}
		};
	}

	/**
	 * \brief Wrapper for key mapping state enum, the least I can do is make sure state modifications occur through a managing class,
	 * and that there exists only one 'current' state, and that it can only be a finite set of possibilities.
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
		DelayManagement::DelayManager<std::chrono::microseconds> LastSentTime{ m_keyDefaults.MICROSECONDS_DELAY_KEYREPEAT };
	public:
		[[nodiscard]] constexpr bool IsRepeating() const noexcept { return m_currentValue == ActionState::KEYREPEAT; }
		[[nodiscard]] constexpr bool IsDown() const noexcept { return m_currentValue == ActionState::KEYDOWN; }
		[[nodiscard]] constexpr bool IsUp() const noexcept { return m_currentValue == ActionState::KEYUP; }
		[[nodiscard]] constexpr bool IsInitialState() const noexcept { return m_currentValue == ActionState::INIT; }
		auto SetDown() noexcept { m_currentValue = ActionState::KEYDOWN; }
		auto SetUp() noexcept { m_currentValue = ActionState::KEYUP; }
		auto SetRepeat() noexcept { m_currentValue = ActionState::KEYREPEAT; }
		auto SetInitial() noexcept { m_currentValue = ActionState::INIT; }
	};

	/**
	 * \brief The exclusivity grouping member is intended to allow the user to add different groups of mappings
	 * that require another mapping from the same group to be "overtaken" or key-up sent before the "over-taking" new mapping
	 * can perform the key-down.
	 */
	struct CBActionMap
	{
		using Fn_t = std::function<void()>;
		using OptFn_t = std::optional<Fn_t>;
		using Delay_t = std::chrono::microseconds;
		using OptDelay_t = std::optional<Delay_t>;
		using GrpVal_t = int;
		using OptGrp_t = std::optional<GrpVal_t>;
		int Vk{};
		bool UsesRepeat{ true };
		OptGrp_t ExclusivityGrouping;
		OptFn_t OnDown;
		OptFn_t OnUp;
		OptFn_t OnRepeat;
		OptDelay_t CustomRepeatDelay; // optional custom delay between key-repeats
		MappingStateManager LastAction; // Last action performed, with get/set methods.
	};
}

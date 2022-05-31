#pragma once
#include "stdafx.h"
#include "STRunner.h"

namespace sds
{
	/// <summary> This is an encapsulation function object with methods for accessing the
	///	data it is operating on. It has a member function operator()() called in the STRunner
	///	thread loop. This is so many objects such as this can be ran on the same thread.
	///	For performance reasons, typically the work thread in this functor will not be doing
	///	much beyond making a system call in a loop and updating something. Polling loops like
	///	this belong in a single thread without a delay to run them all.
	///	Remember that the m_is_enabled member of the base (DataWrapper) toggles on/off the processing of operator()()</summary>
	struct STMousePoller : public STRunner::DataWrapper
	{
	public:
		using StateType = XINPUT_STATE;
		using StateContainerType = std::vector<StateType>;
	private:
		// The number of empty XInput structs before another empty state is added, this
		// helps to keep the path associated with processing them "hot" and not yield()ing it's time
		// when it should be ready to process a valid state.
		const unsigned EMPTY_COUNT{ 10'000u };
		// local ref to settings struct
		const MouseSettings& m_settings;
		// local ref to player info
		const MousePlayerInfo& m_local_player;
		// The buffer holding controller state structures polled.
		StateContainerType m_protectedData;
		// Current count of states in a row that were empty (for the operator()() func)
		unsigned currentEmptyCount{};
		// structure that holds our controller state, added here to avoid creation and destruction in the work func.
		StateType tempState{};
	public:
		virtual ~STMousePoller() override
		{
		}
		explicit STMousePoller(const MousePlayerInfo& pl = {}, const MouseSettings& sett = {}, const LogFnType fn = nullptr)
			: DataWrapper(fn), m_settings(sett), m_local_player(pl)
		{

		}
		void Start() noexcept
		{
			m_is_enabled = true;
		}
		void Stop() noexcept
		{
			m_is_enabled = false;
		}
		/// <summary>Work function ran in the STRunner thread, polls for controller input structs and provides methods to access them.</summary>
		virtual void operator()() override
		{
			// Lambda for adding an element to the container, it restricts the maximum count of unprocessed states in the container.
			auto addElement = [&](const auto& state)
			{
				ScopedLockType addLock(m_mutex);
				if (m_protectedData.size() < m_settings.MAX_STATE_COUNT)
					m_protectedData.push_back(state);
				else if (LogFn != nullptr)
					LogFn("STMousePoller::operator()()::addElement(): State buffer dropping states.");
			};
			// zero controller state struct
			tempState = {};
			// get updated controller state information
			const DWORD error = XInputGetKeystroke(m_local_player.player_id, 0, &tempState);
			if (error == ERROR_SUCCESS)
			{
				addElement(tempState);
			}
			else if (error == ERROR_EMPTY)
			{
				currentEmptyCount++;
				if (currentEmptyCount > EMPTY_COUNT)
				{
					currentEmptyCount = 0;
					// here, an element is added after the count is reached
					// in order to make sure the "hot path" for actually processing states
					// stays active.
					addElement(tempState);
				}
			}
		}
		[[nodiscard]] StateContainerType GetAndClearStates()
		{
			ScopedLockType addLock(m_mutex);
			StateContainerType sct = m_protectedData;
			m_protectedData.clear();
			return sct;
		}
	};
}
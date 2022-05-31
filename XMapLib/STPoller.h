#pragma once
#include "stdafx.h"
#include "STRunner.h"

namespace sds
{
	/// <summary> Class template for an encapsulation function object with methods for accessing the data it is operating on.
	/// <para>It has a member function operator()() called in the STRunner thread loop. This is so many objects such as this can be ran on the same thread.
	///	For performance reasons, typically the work thread in this functor will not be doing much beyond making a system call in a loop and updating something.</para>
	///	<para>Polling loops like this belong in a single thread without a delay to run them all. Remember that the m_is_enabled member of the base (DataWrapper)
	///	toggles on/off the processing of operator()()</para>
	///	<para>**This class template is program specific logic, an input poller to get updated states.</para>
	///	<para>TODO a class that will encapsulate a boolean and a counter, incremented each time an error message is logged,
	///	used to silence an error message after a certain number of calls to avoid spamming an error log or error output.</para>
	///	</summary>
	template<class StructType>
	struct STPoller : public STRunner::DataWrapper
	{
	public:
		using StateContainerType = std::vector<StructType>;
		// Function pointer to hold the fn wrapping the syscall, it should return the struct we are interested in.
		// it returns a std::pair with the first element being the state struct, and the second an error code.
		// An error code of 0 returned should denote no error.
		// An example lambda could be:
		//auto StateFn = [](int pid, XINPUT_STATE& st)
		//{
		//	return XInputGetState(pid, &st);
		//};
		using SysCallType = std::function<int(int, StructType&)>;
	private:
		// This member is used to denote if the state buffer has ever dropped a state,
		// if so this will be set to true and the error log will not be spammed with the same message.
		std::atomic<bool> m_isDroppedStatesErrorState{ false };
		// The number of empty XInput structs before another empty state is added, this
		// helps to keep the path associated with processing them "hot" and not yield()ing it's time
		// when it should be ready to process a valid state.
		const unsigned m_MaxEmptyCount{ 10'000u };
		// The buffer holding controller state structures polled.
		StateContainerType m_protectedData;
		// Current count of states in a row that were empty (for the operator()() func)
		unsigned m_currentEmptyCount{};
		// structure that holds our controller state, added here to avoid creation and destruction in the work func.
		StructType m_tempState{};
		// Function pointer to hold the fn wrapping the syscall, it should return the struct we are interested in.
		// it returns a std::pair with the first element being the state struct, and the second an error code.
		SysCallType m_sysCall;
		// Player ID of the controller we are interested in.
		const int m_PlayerId;
		// Maximum number of controller state structs to hold in the vector before dropping states.
		const int m_MaxStateCount;
	public:
		virtual ~STPoller() override
		{
		}
		explicit STPoller(const SysCallType stateFn, const int pid = 0, const int maxStates = 1'000, const LogFnType fn = nullptr)
			: DataWrapper(fn), m_sysCall(stateFn), m_PlayerId(pid), m_MaxStateCount(maxStates)
		{
			if (stateFn == nullptr && fn != nullptr)
				fn("Exception in STPoller::STPoller(): SysCallType stateFn cannot be nullptr!");
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
			auto addElement = [&](const StructType& state)
			{
				ScopedLockType addLock(m_mutex);
				if (m_protectedData.size() < m_MaxStateCount)
					m_protectedData.push_back(state);
				else if (LogFn != nullptr && !m_isDroppedStatesErrorState)
				{
					LogFn("STPoller::operator()()::addElement(): State buffer dropping states.");
					m_isDroppedStatesErrorState = true;
				}
			};
			// zero controller state struct
			m_tempState = {};
			// get updated controller state information
			const auto errState = m_sysCall(m_PlayerId, m_tempState);
			if (errState == 0)
			{
				addElement(m_tempState);
				return;
			}
			// else, update empty count, possibly add empty element.
			m_currentEmptyCount++;
			if (m_currentEmptyCount > m_MaxEmptyCount)
			{
				m_currentEmptyCount = 0;
				// here, an element is added after the count is reached
				// in order to make sure the "hot path" for actually processing states
				// stays active.
				addElement(m_tempState);
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
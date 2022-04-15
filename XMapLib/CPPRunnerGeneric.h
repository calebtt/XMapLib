#pragma once
#include "stdafx.h"
#include <ranges>
#include <concepts>
namespace sds
{
	/// <summary>Contains using declarations for first two args of the user-supplied lambda function.</summary>
	struct LambdaArgs
	{
		using LambdaArg1 = std::shared_ptr<std::atomic<bool>>;
		using LambdaArg2 = std::mutex;
	};
	/// <summary>All aboard the SFINAE train. It provides facilities for safely accessing data being operated on by a spawned thread,
	///	as well as stopping and starting the running thread.
	///	If you want to use this class, make a function (or lambda function) with parameters
	///	of the form [void] function_name( const LambdaArgs::LambdaArg1 stopCondition, LambdaArgs::LambdaArg2 theMutex, UserType protectedDataYouWantToAccess )
	/// </summary>
	template<typename InternalData>
	requires std::is_default_constructible_v<InternalData>
	class CPPRunnerGeneric
	{
	public:
		using MutexType = LambdaArgs::LambdaArg2;
		using LambdaType = std::function<void(const LambdaArgs::LambdaArg1&, LambdaArgs::LambdaArg2&, InternalData&)>;
		using ScopedLockType = std::lock_guard<MutexType>;

		explicit CPPRunnerGeneric(LambdaType lambdaToRun) : m_lambda(std::move(lambdaToRun)) { }
		CPPRunnerGeneric(const CPPRunnerGeneric& other) = delete;
		CPPRunnerGeneric(CPPRunnerGeneric&& other) = delete;
		CPPRunnerGeneric& operator=(const CPPRunnerGeneric& other) = delete;
		CPPRunnerGeneric& operator=(CPPRunnerGeneric&& other) = delete;
		~CPPRunnerGeneric()
		{
			StopThread();
			ScopedLockType tempLock(this->m_state_mutex);
		}
	protected:
		const LambdaType m_lambda;
		// default constructed type InternalData
		InternalData m_local_state{};
		// shared_ptr is not thread safe with regard to mutating it, but the ref counting copy op is.
		std::shared_ptr<std::atomic<bool>> m_is_stop_requested{ std::make_shared<std::atomic<bool>>() };
		std::unique_ptr<std::thread> m_local_thread{};
		std::mutex m_state_mutex{};
	public:
		/// <summary>Starts running a new thread for the lambda.</summary>
		///	<returns>true on success, false on failure.</returns>
		bool StartThread() noexcept
		{
			if (m_local_thread != nullptr)
				return false;
			m_is_stop_requested = std::make_shared <std::atomic<bool>>(false);
			m_local_thread = std::make_unique<std::thread>(m_lambda, std::cref(m_is_stop_requested), std::ref(m_state_mutex), std::ref(m_local_state));
			return m_local_thread->joinable();
		}
		/// <summary>Returns true if thread is running and stop hasn't been requested.
		/// A detached thread may still be running until it tests the stop condition again.</summary>
		[[nodiscard]] bool IsRunning() const noexcept
		{
			if (m_local_thread != nullptr && m_is_stop_requested != nullptr)
				return m_local_thread->joinable() && !(*m_is_stop_requested);
			return false;
		}
		/// <summary>Non-blocking way to stop a running thread.</summary>
		void RequestStop() noexcept
		{
			if (m_is_stop_requested != nullptr)
			{
				//Get this setting out of the way.
				*m_is_stop_requested = true;
				//If there is a thread obj..
				if (m_local_thread != nullptr)
				{
					// a thread must be joinable() to be detached...
					if (m_local_thread->joinable())
						m_local_thread->detach();
					// clear reference to thread obj, OS handles this
					// so it won't crash when the thread obj is deleted (bc is detached())
					m_local_thread.reset();
				}
				// clear reference to stop condition atomic, thread has it's own reference to it.
				m_is_stop_requested.reset();
			}
		}
		/// <summary>Blocking way to stop a running thread, joins to current thread and waits.</summary>
		void StopThread() noexcept
		{
			if (m_is_stop_requested != nullptr)
			{
				//Get this setting out of the way.
				*m_is_stop_requested = true;
				//If there is a thread obj..
				if (m_local_thread != nullptr)
				{
					if (m_local_thread->joinable())
					{
						//join to wait for thread to stop
						m_local_thread->join();
					}
					m_local_thread.reset();
				}
				// clear reference to stop condition atomic, thread has it's own reference to it.
				m_is_stop_requested.reset();
			}
		}
		/// <summary>Container type function, adds an element to say, a vector.</summary>
		void AddState(const auto& state) requires std::ranges::range<InternalData>
		{
			ScopedLockType tempLock(m_state_mutex);
			m_local_state.push_back(state);
		}
		/// <summary>Container type function, returns copy and clears internal one.</summary>
		auto GetAndClearCurrentStates() requires std::ranges::range<InternalData>
		{
			ScopedLockType tempLock(m_state_mutex);
			auto temp = m_local_state;
			m_local_state.clear();
			return temp;
		}
		/// <summary>Utility function to update the InternalData with mutex locking thread safety.</summary>
		/// <param name="state">InternalData obj to be copied to the internal one.</param>
		void UpdateState(const InternalData& state)
		{
			ScopedLockType tempLock(m_state_mutex);
			m_local_state = state;
		}
		/// <summary>Returns a copy of the internal InternalData obj with mutex locking thread safety.</summary>
		InternalData GetCurrentState()
		{
			ScopedLockType tempLock(m_state_mutex);
			return m_local_state;
		}
	};
}

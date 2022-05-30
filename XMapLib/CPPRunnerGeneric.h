#pragma once
#include "stdafx.h"
#include <ranges>
#include <memory>
namespace sds
{
	/// <summary>Contains using declarations for first two args of the user-supplied lambda function.</summary>
	struct LambdaArgs
	{
		using MutexType = std::mutex;
		using StopCondType = std::atomic<bool>;
		using MutexPointerType = std::shared_ptr<MutexType>;
		using StopCondPointerType = std::shared_ptr<StopCondType>;
		using LambdaArg1 = StopCondPointerType;
		using LambdaArg2 = MutexPointerType;
	};
	/// <summary>All aboard the SFINAE train. It provides facilities for safely accessing data being operated on by a spawned thread,
	///	as well as stopping and starting the running thread.
	///	If you want to use this class, make a function (or lambda function) with parameters
	///	of the form [void] function_name( const LambdaArgs::LambdaArg1 stopCondition, LambdaArgs::LambdaArg2 theMutex, UserType protectedDataYouWantToAccess )
	///	**
	///	Data is owned and operated on by a running thread, and if stop() is requested, that data will no longer be reachable.
	///	TODO detach()ed thread, mutex, and data objects should be saved into a range and upon destruction join()ed to ensure they are disposed of
	///	before the termination of the program.
	///	TODO need a static thread runner type that accepts multiple lambda functions and calls them in sequence.
	///	Best solution for input polling (all on a single thread).
	/// </summary>
	template<typename InternalData>
	requires std::is_default_constructible_v<InternalData>
	class CPPRunnerGeneric
	{
		/// <summary> Aliases make_shared to make it easier to update for changing to a different
		///	smart pointer type. </summary>
		template<typename T>
		inline static const auto MakeSmart = std::make_shared<T>;
		//template<typename T>
		//const auto MakeSmart = []<typename ... S>(S&& ... args) { return std::make_shared<T>(std::forward<S>(args)...); };
		/// <summary> Returns true if smart pointers are null. </summary>
		[[nodiscard]] bool ArePointersNull() const noexcept
		{
			// if any of these are in mixed states, a user tried to use
			// the class functions asynchronously and the invariants are invalidated
			return !(m_is_stop_requested != nullptr
				&& m_local_thread != nullptr
				&& m_state_mutex != nullptr
				&& m_local_state != nullptr);
		}
	public:
		using MutexType = LambdaArgs::MutexType;
		using MutexPointerType = LambdaArgs::MutexPointerType;
		using DataPointerType = std::shared_ptr<InternalData>;
		using LambdaType = std::function<void(LambdaArgs::LambdaArg1, LambdaArgs::LambdaArg2, DataPointerType)>;
		using ScopedLockType = std::lock_guard<MutexType>;
		using StopCondType = LambdaArgs::StopCondType;
		using StopCondPointerType = LambdaArgs::StopCondPointerType;

		explicit CPPRunnerGeneric(LambdaType lambdaToRun) : m_lambda(std::move(lambdaToRun)) { }
		CPPRunnerGeneric(const CPPRunnerGeneric& other) = delete;
		CPPRunnerGeneric(CPPRunnerGeneric&& other) = delete;
		CPPRunnerGeneric& operator=(const CPPRunnerGeneric& other) = delete;
		CPPRunnerGeneric& operator=(CPPRunnerGeneric&& other) = delete;
		~CPPRunnerGeneric()
		{
			StopThread();
			if(m_state_mutex != nullptr)
				ScopedLockType endLock(*m_state_mutex);
		}
	protected:
		const LambdaType m_lambda;
		// default constructed type InternalData
		DataPointerType m_local_state{};
		// shared_ptr is not thread safe with regard to mutating it, but the ref counting copy op is.
		StopCondPointerType m_is_stop_requested{};
		MutexPointerType m_state_mutex{};
		std::unique_ptr<std::thread> m_local_thread{};
	public:
		/// <summary>Starts running a new thread for the lambda.</summary>
		///	<returns>true on success, false on failure.</returns>
		bool StartThread() noexcept
		{
			// if any of these are in mixed states, a user tried to use
			// the class functions asynchronously and the invariants are invalidated
			if (ArePointersNull())
			{
				// create the thread-specific objects that would enable it to run while detached and a new thread
				// on a new piece of data can be started directly after.
				m_is_stop_requested = MakeSmart<StopCondType>();
				*m_is_stop_requested = false;
				m_state_mutex = MakeSmart<MutexType>();
				m_local_state = MakeSmart<InternalData>();
				// start the thread and copy the smart pointers
				m_local_thread = std::make_unique<std::thread>(m_lambda, (m_is_stop_requested), (m_state_mutex), (m_local_state));
				return m_local_thread->joinable();
			}
			return false;
		}
		/// <summary>Returns true if thread is running and stop has not been requested.
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
			// if any of these are in mixed states, a user tried to use
			// the class functions asynchronously and the invariants are invalidated
			if(!ArePointersNull())
			{
				// do some work and reset the class data member pointers.
				*m_is_stop_requested = true;
				m_is_stop_requested.reset();
				m_local_state.reset();
				m_state_mutex.reset();
				// a thread must be joinable() to be detached...
				if (m_local_thread->joinable())
					m_local_thread->detach();
				m_local_thread.reset();
			}
		}
		/// <summary>Blocking way to stop a running thread, joins to current thread and waits.</summary>
		void StopThread() noexcept
		{
			// if any of these are in mixed states, a user tried to use
			// the class functions asynchronously and the invariants are invalidated
			if (!ArePointersNull())
			{
				// do some work and reset the class data member pointers.
				*m_is_stop_requested = true;
				m_is_stop_requested.reset();
				m_local_state.reset();
				m_state_mutex.reset();
				// a thread must be joinable() to be joined...
				if (m_local_thread->joinable())
					m_local_thread->join();
				m_local_thread.reset();
			}
		}
		/// <summary>Container type function, adds an element to say, a vector. State updates will not
		/// occur if stop has been requested.</summary>
		void AddState(const auto& state) requires std::ranges::range<InternalData>
		{
			if (!ArePointersNull())
			{
				if (!(*m_is_stop_requested))
				{
					ScopedLockType tempLock(*m_state_mutex);
					m_local_state->push_back(state);
				}
			}
		}
		/// <summary>Container type function, returns copy and clears internal one. If stop has been requested,
		/// returns default constructed InternalData</summary>
		[[nodiscard]] InternalData GetAndClearCurrentStates() const requires std::ranges::range<InternalData>
		{
			if (!ArePointersNull())
			{
				if (!(*m_is_stop_requested))
				{
					ScopedLockType tempLock(*m_state_mutex);
					auto temp = *m_local_state;
					m_local_state->clear();
					return temp;
				}
			}
			Utilities::LogError("Error in CPPRunnerGeneric::GetAndClearCurrentStates(): pointers were null, or stop was requested. Returned default constructed InternalData");
			return InternalData{};
		}
		/// <summary>Utility function to update the InternalData with mutex locking thread safety.
		/// If stop has been requested, does nothing. </summary>
		/// <param name="state">InternalData obj to be copied to the internal one.</param>
		void UpdateState(const InternalData& state)
		{
			if (!ArePointersNull())
			{
				if (!(*m_is_stop_requested))
				{
					ScopedLockType tempLock(*m_state_mutex);
					*m_local_state = state;
				}
			}
		}
		/// <summary>Returns a copy of the internal InternalData obj with mutex locking thread safety.
		/// If stop has been requested, returns default constructed InternalData. </summary>
		InternalData GetCurrentState()
		{
			if (!ArePointersNull())
			{
				if (!(*m_is_stop_requested))
				{
					ScopedLockType tempLock(*m_state_mutex);
					return *m_local_state;
				}
			}
			return InternalData{};
		}
	};
}

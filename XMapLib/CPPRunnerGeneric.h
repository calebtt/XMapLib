#pragma once
#include <mutex>
#include <atomic>
#include <ranges>
#include <memory>
#include <functional>
#include <algorithm>

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
	///	as well as stopping and starting the running thread. If you want to use this class, make a function (or lambda function) with parameters
	///	of the form <code> void function_name( const LambdaArgs::LambdaArg1 stopConditionPtr, LambdaArgs::LambdaArg2 theMutexPtr, UserType protectedDataYouWantToAccess ) </code>
	/// </summary>
	template<typename InternalData> requires std::is_default_constructible_v<InternalData>
	class CPPRunnerGeneric
	{
		///<summary> Factory func for making shared smart pointer type. (makes it easier to change to a new type if desired.) </summary>
		template<typename T>
		auto MakeSmart(auto ... args)
		{
			return std::make_shared<T>(args...);
		}
		/// <summary> Returns true if smart pointers are null. </summary>
		[[nodiscard]] bool ArePointersNull() const noexcept
		{
			// if any of these are in mixed states, a user tried to use
			// the class functions asynchronously and the invariants are invalidated
			return !(m_current_data_pack.m_is_stop_requested != nullptr
				&& m_current_data_pack.m_local_thread != nullptr
				&& m_current_data_pack.m_state_mutex != nullptr
				&& m_current_data_pack.m_local_data != nullptr);
		}
		/// <summary> If threads in the dead session buffer are join()able it will join() them
		/// in order to prep them for erasure. Blocking operation. </summary>
		void TidyDeadSessionBuffer()
		{
			if (m_previous_runs_buffer.empty())
				return;
			//iterate vector, join()ing where able
			for (auto& pack : m_previous_runs_buffer)
			{
				if (pack.m_local_thread != nullptr)
				{
					if (pack.m_local_thread->joinable())
						pack.m_local_thread->join();
				}
			}
		}
		/// <summary> If dead sessions data packs in the buffer are not joinable() they will be erased. </summary>
		void EraseDeadSessions()
		{
			//erase non-join()able data packs
			std::erase_if(m_previous_runs_buffer, [](SessionDataPack& p)
				{
					if (p.m_local_thread == nullptr)
						return true;
					return !p.m_local_thread->joinable();
				});
		}
		/// <summary> Adds a session data pack to the buffer of previously ran data packs.
		///	May block while running cleanup routine to limit maximum size of buffer. </summary>
		/// <param name="sess"></param>
		void AddSessionDataPackToBuffer(const auto sess)
		{
			if (m_previous_runs_buffer.size() > MaximumBufferSize)
			{
				TidyDeadSessionBuffer();
				EraseDeadSessions();
			}
			m_previous_runs_buffer.emplace_back(sess);
		}
	public:
		using MutexType = LambdaArgs::MutexType;
		using MutexPointerType = LambdaArgs::MutexPointerType;
		using DataPointerType = std::shared_ptr<InternalData>;
		using LambdaType = std::function<void(LambdaArgs::LambdaArg1, LambdaArgs::LambdaArg2, DataPointerType)>;
		using ScopedLockType = std::lock_guard<MutexType>;
		using StopCondType = LambdaArgs::StopCondType;
		using StopCondPointerType = LambdaArgs::StopCondPointerType;
		using LogFnType = std::function<void(std::string)>;

		explicit CPPRunnerGeneric(LambdaType lambdaToRun, const LogFnType logFn = nullptr)
			: m_lambda(std::move(lambdaToRun)),
			m_logFn(logFn)
		{ }
		CPPRunnerGeneric(const CPPRunnerGeneric& other) = delete;
		CPPRunnerGeneric(CPPRunnerGeneric&& other) = delete;
		CPPRunnerGeneric& operator=(const CPPRunnerGeneric& other) = delete;
		CPPRunnerGeneric& operator=(CPPRunnerGeneric&& other) = delete;
		~CPPRunnerGeneric()
		{
			StopThread();
			//ensure previous thread not still running too.
			if (m_previous_data_pack.m_local_thread != nullptr)
			{
				if (m_previous_data_pack.m_local_thread->joinable())
					m_previous_data_pack.m_local_thread->join();
			}
		}
	protected:
		struct SessionDataPack
		{
			//pointer to stop condition atomic.
			StopCondPointerType m_is_stop_requested;
			//the associated thread that was (virtually) detached.
			std::shared_ptr<std::jthread> m_local_thread;
			//pointer to the mutex associated with the (virtual) detachment of the thread.
			MutexPointerType m_state_mutex;
			//pointer to the internal data result after the thread was terminated.
			DataPointerType m_local_data;
		};
	protected:
		//maximum count of session data packs to be stored in the buffer
		//before a cleanup routine is called and a blocking operation is performed
		//to join() them back before removal.
		static constexpr size_t MaximumBufferSize{ 100 };
		const LambdaType m_lambda;
		const LogFnType m_logFn;
		//Session data pack for the current thread of execution.
		SessionDataPack m_current_data_pack;
		//Session data pack with previous run's data.
		SessionDataPack m_previous_data_pack;
		//Session data for previous runs, the thread obj. might need join()ed
		//back to the main thread before destruction as it is possible to request
		//a stop instead of block and wait for it.
		std::vector<SessionDataPack> m_previous_runs_buffer;
	public:
		/// <summary>Starts running a new thread for the lambda if one does not exist.</summary>
		///	<returns>true on success, false on failure.</returns>
		bool StartThread() noexcept
		{
			// if any of these are in mixed states, a user tried to use
			// the class functions asynchronously and the invariants are invalidated
			if (ArePointersNull())
			{
				// create the thread-specific objects that would enable it to run while (virtually) detached and a new thread
				// on a new piece of data can be started directly after.
				m_current_data_pack.m_is_stop_requested = MakeSmart<StopCondType>();
				*m_current_data_pack.m_is_stop_requested = false;
				m_current_data_pack.m_state_mutex = MakeSmart<MutexType>();
				m_current_data_pack.m_local_data = MakeSmart<InternalData>();
				// start the thread and copy the smart pointers
				m_current_data_pack.m_local_thread = MakeSmart<std::jthread>(m_lambda, (m_current_data_pack.m_is_stop_requested), (m_current_data_pack.m_state_mutex), (m_current_data_pack.m_local_data));
				return m_current_data_pack.m_local_thread->joinable();
			}
			return false;
		}
		/// <summary>Returns true if thread is running and stop has not been requested.
		/// A detached thread may still be running until it tests the stop condition again.</summary>
		[[nodiscard]] bool IsRunning() const noexcept
		{
			if (m_current_data_pack.m_local_thread != nullptr && m_current_data_pack.m_is_stop_requested != nullptr)
				return m_current_data_pack.m_local_thread->joinable() && !(*m_current_data_pack.m_is_stop_requested);
			return false;
		}
		/// <summary> Potentially Non-blocking way to stop a running thread. A large buffer of previous
		/// requested stops may trigger a cleanup routine. </summary>
		void RequestStop() noexcept
		{
			// if any of these are in mixed states, a user tried to use
			// the class functions asynchronously and the invariants are invalidated
			if (!ArePointersNull())
			{
				// store a copy of (pointers to) the previous run's associated data.
				AddSessionDataPackToBuffer(m_previous_data_pack);
				m_previous_data_pack = m_current_data_pack;
				// do some work and reset the pointers.
				*m_current_data_pack.m_is_stop_requested = true;
				m_current_data_pack.m_is_stop_requested.reset();
				m_current_data_pack.m_local_data.reset();
				m_current_data_pack.m_state_mutex.reset();
				//here we are not detaching the thread of execution from the std::thread obj
				//because we want to be able to join() it before destruction.
				m_current_data_pack.m_local_thread.reset();
			}
		}
		/// <summary>Blocking way to stop a running thread, joins to current thread and waits.</summary>
		void StopThread() noexcept
		{
			// if any of these are in mixed states, a user tried to use
			// the class functions asynchronously and the invariants are invalidated
			if (!ArePointersNull())
			{
				// store a copy of (pointers to) the previous run's associated data.
				AddSessionDataPackToBuffer(m_previous_data_pack);
				m_previous_data_pack = m_current_data_pack;
				// do some work and reset the pointers.
				*m_current_data_pack.m_is_stop_requested = true;
				m_current_data_pack.m_is_stop_requested.reset();
				m_current_data_pack.m_local_data.reset();
				m_current_data_pack.m_state_mutex.reset();
				// a thread must be joinable() to be joined...
				if (m_current_data_pack.m_local_thread->joinable())
					m_current_data_pack.m_local_thread->join();
				m_current_data_pack.m_local_thread.reset();
			}
		}
		/// <summary>Container type function, adds an element to say, a vector. State updates will not
		/// occur if stop has been requested.</summary>
		///	<returns> true on successfully added state, false otherwise. </returns>
		bool AddState(const auto& state) requires std::ranges::range<InternalData>
		{
			if (!ArePointersNull())
			{
				if (!(*m_current_data_pack.m_is_stop_requested))
				{
					ScopedLockType tempLock(*m_current_data_pack.m_state_mutex);
					m_current_data_pack.m_local_data->push_back(state);
					return true;
				}
			}
			return false;
		}
		/// <summary>Container type function, returns copy and clears internal one. If stop has been requested,
		/// returns default constructed InternalData</summary>
		[[nodiscard]] InternalData GetAndClearCurrentStates() const requires std::ranges::range<InternalData>
		{
			if (!ArePointersNull())
			{
				if (!(*m_current_data_pack.m_is_stop_requested))
				{
					ScopedLockType tempLock(*m_current_data_pack.m_state_mutex);
					auto temp = *m_current_data_pack.m_local_data;
					m_current_data_pack.m_local_data->clear();
					return temp;
				}
			}
			if (m_logFn != nullptr)
				m_logFn("Error in CPPRunnerGeneric::GetAndClearCurrentStates(): pointers were null, or stop was requested. Returned default constructed InternalData");
			return InternalData{};
		}
		/// <summary>Utility function to update the InternalData with mutex locking thread safety.
		/// If stop has been requested, does nothing. </summary>
		/// <param name="state">InternalData obj to be copied to the internal one.</param>
		///	<returns> true on successfully added state, false otherwise. </returns>
		bool UpdateState(const InternalData& state)
		{
			if (!ArePointersNull())
			{
				if (!(*m_current_data_pack.m_is_stop_requested))
				{
					ScopedLockType tempLock(*m_current_data_pack.m_state_mutex);
					*m_current_data_pack.m_local_data = state;
					return true;
				}
			}
			return false;
		}
		/// <summary>Returns a copy of the internal InternalData obj with mutex locking thread safety.
		/// If stop has been requested (or completed), returns default constructed InternalData. </summary>
		InternalData GetCurrentState()
		{
			if (!ArePointersNull())
			{
				if (!(*m_current_data_pack.m_is_stop_requested))
				{
					ScopedLockType tempLock(*m_current_data_pack.m_state_mutex);
					return *m_current_data_pack.m_local_data;
				}
			}
			return InternalData{};
		}
		/// <summary> Returns a copy of the InternalData from a previous Start() and Stop() of the object.
		///	If called while running but before Stop() or RequestStop() has been called,
		///	returns default constructed InternalData. </summary>
		/// <typeparam name="InternalData">The InternalData from the previous run, or default constructed InternalData if not available.</typeparam>
		InternalData GetPreviousState()
		{
			if(m_previous_data_pack.m_local_data != nullptr && m_previous_data_pack.m_state_mutex != nullptr)
			{
				ScopedLockType tempLock(*m_previous_data_pack.m_state_mutex);
				return *m_previous_data_pack.m_local_data;
			}
			return InternalData{};
		}
	};
}

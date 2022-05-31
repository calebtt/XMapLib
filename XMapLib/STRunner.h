#pragma once
#include "stdafx.h"

namespace sds
{
	/// <summary>STRunner provides facilities for running multiple functions (with their own data and access synchronization) on
	///	a single thread, as well as stopping and starting the thread.
	///	If you want to use this class, make a function object that derives from DataWrapper and pass a shared_ptr to it into AddDataWrapper(...)
	///	The user provided function object deriving from DataWrapper will need to provide it's own data set to operate on, as well as the
	///	synchronization for accessing it. A mutex for this purpose is provided by DataWrapper, and the calling of the function can be temporarily
	///	disabled via the m_is_enabled atomic bool in DataWrapper.
	/// </summary>
	class STRunner
	{
	public:
		/// <summary> <para>DataWrapper is a base class for function objects that
		///	manage their own access to the data they want protected while running
		///	on another thread. Operator() is overloaded in derived classes to encapsulate
		///	the code to be ran concurrently, and the user should provide concurrent access
		///	methods such as "GetCopyOfData()" or "ClearData()".</para>
		/// <para>Choosing this approach allows many of these function objects to be ran
		/// on a single extra thread, it is appropriate for polling functions that must be
		/// ran indefinitely with no loop delay, but do little more than make system calls
		/// and report the information to somewhere else.</para> </summary>
		class DataWrapper
		{
		public: /* Giant list of using declaration and other aliases. */
			using MutType = std::mutex; // The type of the mutex used for general access coordination.
			using EnabledCondType = std::atomic<bool>; // Alias for stop condition type, not pointer wrapped.
			using ScopedLockType = std::lock_guard<MutType>; // Alias for chosen scoped lock type.
			using LogFnType = std::function<void(const char* st)>; // Alias for logging function pointer type.
		protected: /* Section for used data members */
			const LogFnType LogFn;
			MutType m_mutex;
			EnabledCondType m_is_enabled{ true };
		public:
			//Constructor, LogFnType is an optional logging function that accepts const char* as the only arg.
			explicit DataWrapper(const LogFnType fn = nullptr) : LogFn(fn) { }
			// Pure virtual operator() overload, to be overridden in derived classes.
			virtual void operator()() = 0;
			virtual ~DataWrapper() = default;
			virtual bool IsEnabled() const noexcept { return m_is_enabled; }
		};
	public:
		// Alias for thread class type.
		using ThreadType = std::thread;
		// The type of the mutex used for general access coordination.
		using MutexType = std::mutex;
		// Alias for stop condition type, not pointer wrapped.
		using StopCondType = std::atomic<bool>;
		// Alias for chosen scoped lock type.
		using ScopedLockType = std::lock_guard<MutexType>;
		// Alias for logging function pointer type.
		using LogFnType = std::function<void(const char* st)>;
		// Alias for container that maps a unique identifier to a function pointer.
		using FnListType = std::vector<std::shared_ptr<DataWrapper>>;

		STRunner() = default;
		STRunner(const STRunner& other) = delete;
		STRunner(STRunner&& other) = delete;
		STRunner& operator=(const STRunner& other) = delete;
		STRunner& operator=(STRunner&& other) = delete;
		~STRunner()
		{
			StopThread();
		}
	protected:
		// Callback func passed in via ctor, used for reporting error messages.
		const LogFnType LoggingCallback;
		// The singular thread each instance of the class will have their user-supplied work fn executing on.
		std::unique_ptr<ThreadType> m_local_thread;
		// Atomic stop condition bool.
		StopCondType m_is_stop_requested{ false };
		// Container of work function objects to be called.
		FnListType m_function_list;
		// Mutex for modifying the function list.
		MutexType m_functionMutex;
	protected:
		void workFunction() noexcept
		{
			using namespace std::chrono;
			//while static thread stop not requested
			while (!m_is_stop_requested)
			{
				bool foundFunction = false;
				ScopedLockType tempLock(m_functionMutex);
				//loop through function list and call operator() if enabled
				for (auto& elem : m_function_list)
				{
					if (elem->IsEnabled())
					{
						(*elem)();
						foundFunction = true;
					}
				}
				if (!foundFunction)
					std::this_thread::sleep_for(milliseconds(10));
			}
		}
	public:
		void AddDataWrapper(const std::shared_ptr<DataWrapper>& dw)
		{
			ScopedLockType tempLock(m_functionMutex);
			m_function_list.emplace_back(dw);
		}
		/// <summary>Starts running a new thread for the lambda.</summary>
		///	<returns>true on success, false on failure.</returns>
		bool StartThread() noexcept
		{
			if (m_local_thread == nullptr)
			{
				// start the thread and copy the smart pointers
				m_local_thread = std::make_unique<ThreadType>([this]() { workFunction(); });
				return m_local_thread->joinable();
			}
			return false;
		}
		/// <summary>Returns true if thread is running and stop has not been requested.</summary>
		[[nodiscard]] bool IsRunning() const noexcept
		{
			if (m_local_thread != nullptr)
				return m_local_thread->joinable() && !m_is_stop_requested;
			return false;
		}
		/// <summary>Blocking way to stop a running thread, joins to current thread and waits.</summary>
		void StopThread() noexcept
		{
			if (m_local_thread != nullptr)
			{
				// do some work and reset the class data member pointers.
				m_is_stop_requested = true;
				// a thread must be joinable() to be joined...
				if (m_local_thread->joinable())
					m_local_thread->join();
				m_local_thread.reset();
			}
		}
	};
}

#pragma once
#include "stdafx.h"
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>
#include "CPPRunnerGeneric.h"
#include "STDataWrapper.h"

namespace sds
{
	/// <summary><para><c>STRunner</c> provides facilities for running multiple functions (with their own data and access synchronization) on
	///	a single thread, as well as stopping and starting the thread.</para>
	///	<para>If you want to use this class, make a function object that derives from <c>STDataWrapper</c> and pass a <c>shared_ptr</c>
	///	to it into <c>AddDataWrapper(...)</c> The user provided function object deriving from DataWrapper will need to provide it's own data set to operate on, as well as the
	///	synchronization for accessing it. A <c>mutex</c> for this purpose is provided by <c>STDataWrapper</c>, and the calling of the function can be temporarily
	///	disabled via the <c>m_is_enabled</c> atomic bool in <c>STDataWrapper</c>.</para>
	///	<para>TODO add benchmarking timers to average each separate function object's execution time.
	///	</para>
	/// </summary>
	class STRunner
	{
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
		using LogFnType = std::function<void(std::string)>;
		// Alias for container that maps a unique identifier to a function pointer.
		using FnListType = std::vector<std::shared_ptr<STDataWrapper>>;

		auto GetLambda()
		{
			return [this](const auto stopCondition, const auto mut, auto protectedData) { workThread(stopCondition, mut, protectedData); };
		}
		STRunner(const LogFnType logFn = nullptr) : m_threadRunner(GetLambda(), logFn) { }
		STRunner(const STRunner& other) = delete;
		STRunner(STRunner&& other) = delete;
		STRunner& operator=(const STRunner& other) = delete;
		STRunner& operator=(STRunner&& other) = delete;
		~STRunner()
		{
			StopThread();
		}
	protected:
		// thread runner manager
		sds::CPPRunnerGeneric<FnListType> m_threadRunner;
	protected:
		void workThread(const auto stopCondition, const auto mut, auto protectedData)
		{
			using namespace std::chrono;
			//boolean denoting when a function object in the pool is enabled,
			//used to add a tiny loop delay when all objects in the pool are disabled.
			bool foundFunction = false;
			//while static thread stop not requested
			while (!(*stopCondition))
			{
				//local scope for scoped mutex.
				{
					foundFunction = false;
					ScopedLockType tempLock(*mut);
					//loop through function list and call operator() if enabled
					for (const std::shared_ptr<sds::STDataWrapper> &elem : *protectedData)
					{
						if (elem->IsEnabled())
						{
							(*elem)();
							foundFunction = true;
						}
					}
				}
				if (!foundFunction)
					std::this_thread::sleep_for(milliseconds(10));
			}
		}
	public:
		/// <summary> Adds a smart pointer to the DataWrapper base class, or a derived type, to
		///	the internal function list for processing on the thread pool. </summary>
		///	<remarks>NOTE: the thread must be started before adding DataWrappers!</remarks>
		/// <param name="dw">smart pointer to DataWrapper or derived class. </param>
		///	<returns> true on successfully added DataWrapper, false otherwise. </returns>
		bool AddDataWrapper(const std::shared_ptr<STDataWrapper>& dw)
		{
			return m_threadRunner.AddState(dw);
		}
		void ClearAllWrappers()
		{
			const auto temp = m_threadRunner.GetAndClearCurrentStates();
		}
		[[nodiscard]] FnListType GetWrapperBuffer()
		{
			return m_threadRunner.GetCurrentState();
		}
		/// <summary>Starts running a new thread for the lambda if one does not exist.</summary>
		///	<returns>true on success, false on failure.</returns>
		bool StartThread()
		{
			return m_threadRunner.StartThread();
		}
		/// <summary>Returns true if thread is running and stop has not been requested.</summary>
		[[nodiscard]] bool IsRunning() const noexcept
		{
			return m_threadRunner.IsRunning();
		}
		/// <summary>Blocking way to stop a running thread, joins to current thread and waits.</summary>
		void StopThread() noexcept
		{
			m_threadRunner.StopThread();
		}
	};
}

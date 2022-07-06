#pragma once
#include "stdafx.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <ranges>
#include <concepts>
#include <functional>

namespace sds
{
	/// <summary> All aboard the SFINAE train. It provides facilities for safely accessing data being operated on by a spawned thread,
	///	as well as stopping and starting the running thread. If you want to use this class, make a function (or lambda function) with parameters
	///	of the form <code> void function_name( auto &stopConditionBool, auto &theMutex, UserType &protectedDataYouWantToAccess ) </code>
	/// </summary>
	template<typename InternalData, typename LogFnType = std::function<void(std::string)>>
	requires std::is_default_constructible_v<InternalData>
	class AsyncDataRunner
	{
	public:
		/// <summary> Contains using declaration type aliases for the args of the
		/// user-supplied lambda function, and some helpers. </summary>
		struct LambdaArgs
		{
			using ThreadObjType = std::jthread;
			using MutexType = std::mutex;
			using StopCondType = std::atomic<bool>;
			using Ar1StopCond = StopCondType&;
			using Ar2Mut = MutexType&;
			using Ar3Data = InternalData&;
		};
	private:
		// Using declarations section
		using DataPointerType = std::shared_ptr<InternalData>;
		using LambdaType = std::function<void(typename LambdaArgs::Ar1StopCond, typename LambdaArgs::Ar2Mut, typename LambdaArgs::Ar3Data)>;
		using ScopedLockType = std::scoped_lock<typename LambdaArgs::MutexType>;

		/// <summary> Data pack for a thread run's session information. </summary>
		struct SessionDataPack
		{
			using TOType = typename LambdaArgs::ThreadObjType;
			using SStopAtomic = typename LambdaArgs::StopCondType;
			using SMutexType = typename LambdaArgs::MutexType;
			using SThreadType = std::shared_ptr<typename LambdaArgs::ThreadObjType>;

			SThreadType m_local_thread;
			SStopAtomic m_is_stop_requested{ false };
			SMutexType m_state_mutex{};
			InternalData m_local_data{};

			/// <summary> constructs the thread obj with lambda fn </summary>
			void InitThread(auto lambdaFn) { m_local_thread = MakeSmart<TOType>(std::move(lambdaFn), std::ref(m_is_stop_requested), std::ref(m_state_mutex), std::ref(m_local_data)); }
			///<summary> Factory func for making shared smart pointer type. (makes it easier to change to a new type if desired.) </summary>
			template<typename T>
			auto MakeSmart(auto ... args) { return std::make_shared<T>(args...); }
		};
	public:
		explicit AsyncDataRunner(LambdaType lambdaToRun, const LogFnType logFn = nullptr)
			: m_lambda(std::move(lambdaToRun)),
			m_logFn(logFn)
		{ }
		AsyncDataRunner(const AsyncDataRunner& other) = delete;
		AsyncDataRunner(AsyncDataRunner&& other) = delete;
		AsyncDataRunner& operator=(const AsyncDataRunner& other) = delete;
		AsyncDataRunner& operator=(AsyncDataRunner&& other) = delete;
		~AsyncDataRunner()
		{
			StopThread();
		}
	private:
		const LambdaType m_lambda;
		const LogFnType m_logFn;
		//Session data pack for the current thread of execution.
		SessionDataPack m_current_data_pack;
	public:
		/// <summary>Starts running a new thread for the lambda if one does not exist.</summary>
		///	<returns>true on success, false on failure.</returns>
		bool StartThread() noexcept
		{
			if (m_current_data_pack.m_local_thread == nullptr)
			{
				m_current_data_pack.m_is_stop_requested = false;
				m_current_data_pack.InitThread(m_lambda);
				return m_current_data_pack.m_local_thread->joinable();
			}
			return false;
		}
		/// <summary> Returns true if thread is running. </summary>
		[[nodiscard]]
		bool IsRunning() const noexcept
		{
			if (m_current_data_pack.m_local_thread != nullptr)
				return m_current_data_pack.m_local_thread->joinable() && !m_current_data_pack.m_is_stop_requested;
			return false;
		}
		/// <summary> Blocking way to stop a running thread, joins to current thread and waits. </summary>
		void StopThread() noexcept
		{
			if (m_current_data_pack.m_local_thread != nullptr)
			{
				m_current_data_pack.m_is_stop_requested = true;
				m_current_data_pack.m_local_thread.reset();
			}
		}
		/// <summary> Container type function, adds an element to say, a vector. </summary>
		void AddState(const auto& state) requires std::ranges::range<InternalData>
		{
			ScopedLockType tempLock(m_current_data_pack.m_state_mutex);
			m_current_data_pack.m_local_data.emplace_back(state);
		}
		/// <summary> Container type function, returns copy and clears internal one. </summary>
		[[nodiscard]]
		InternalData GetAndClearCurrentStates() requires std::ranges::range<InternalData>
		{
			ScopedLockType tempLock(m_current_data_pack.m_state_mutex);
			auto temp = m_current_data_pack.m_local_data;
			m_current_data_pack.m_local_data.clear();
			return temp;
		}
		/// <summary> Container type function, clears internal state container. </summary>
		void ClearCurrentStates() requires std::ranges::range<InternalData>
		{
			ScopedLockType tempLock(m_current_data_pack.m_state_mutex);
			m_current_data_pack.m_local_data.clear();
		}
		/// <summary> Utility function to update the InternalData with mutex locking thread safety. </summary>
		/// <param name="state"> InternalData obj to be copied to the internal one. </param>
		void UpdateState(const InternalData& state)
		{
			ScopedLockType tempLock(m_current_data_pack.m_state_mutex);
			m_current_data_pack.m_local_data = state;
		}
		/// <summary> Returns a copy of the internal InternalData obj with mutex locking thread safety. </summary>
		[[nodiscard]]
		InternalData GetCurrentState() noexcept
		{
			ScopedLockType tempLock(m_current_data_pack.m_state_mutex);
			return m_current_data_pack.m_local_data;
		}
	};
}



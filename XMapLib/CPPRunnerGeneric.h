#pragma once
#include "stdafx.h"
#include <ranges>
#include <concepts>
namespace sds
{
	/// <summary>Contains using declarations for first two args of the user-supplied lambda function.</summary>
	struct LambdaArgs
	{
		using LambdaArg1 = std::atomic<bool>;
		using LambdaArg2 = std::mutex;
	};
	/// <summary>All aboard the SFINAE train</summary>
	template<typename InternalData>
	requires std::is_default_constructible_v<InternalData>
	class CPPRunnerGeneric
	{
	public:
		using LambdaType = std::function<void(std::atomic<bool>&, std::mutex&, InternalData&)>;
		using ScopedLockType = std::lock_guard<std::mutex>;

		CPPRunnerGeneric(LambdaType lambdaToRun) : m_lambda(std::move(lambdaToRun)) { }
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
		InternalData m_local_state{}; // default constructed type InternalData
		std::atomic<bool> m_is_stop_requested{ false };
		std::unique_ptr<std::thread> m_local_thread{};
		std::mutex m_state_mutex{};
	public:
		/// <summary>Starts running a new thread for the lambda.</summary>
		///	<returns>true on success, false on failure.</returns>
		bool StartThread() noexcept
		{
			if (m_local_thread != nullptr)
				return false;
			m_is_stop_requested = false;
			m_local_thread = std::make_unique<std::thread>(m_lambda, std::ref(m_is_stop_requested), std::ref(m_state_mutex), std::ref(m_local_state));
			return m_local_thread->joinable();
		}
		/// <summary>Returns true if thread is running.</summary>
		bool IsRunning() const noexcept
		{
			if (m_local_thread != nullptr)
				return m_local_thread->joinable() && !m_is_stop_requested;
			return false;
		}
		/// <summary>Non-blocking way to stop a running thread.</summary>
		void RequestStop() noexcept
		{
			//Get this setting out of the way.
			this->m_is_stop_requested = true;
			//If there is a thread obj..
			if (this->m_local_thread != nullptr)
			{
				this->m_local_thread->detach();
				this->m_local_thread.reset();
			}
		}
		/// <summary>Blocking way to stop a running thread, joins to current thread and waits.</summary>
		void StopThread() noexcept
		{
			//Get this setting out of the way.
			this->m_is_stop_requested = true;
			//If there is a thread obj..
			if (this->m_local_thread != nullptr)
			{
				if (this->m_local_thread->joinable())
				{
					//join to wait for thread to stop, then reset to a nullptr.
					this->m_local_thread->join();
					this->m_local_thread.reset();
				}
				else
					//if it is not joinable, set to nullptr
					this->m_local_thread.reset();
			}
		}
		/// <summary>Container type function, adds an element to say, a vector.</summary>
		void AddState(const auto& state) requires std::ranges::range<InternalData>
		{
			ScopedLockType tempLock(this->m_state_mutex);
			this->m_local_state.push_back(state);
		}
		/// <summary>Container type function, returns copy and clears internal one.</summary>
		auto GetAndClearCurrentStates() requires std::ranges::range<InternalData>
		{
			ScopedLockType tempLock(this->m_state_mutex);
			auto temp = this->m_local_state;
			this->m_local_state.clear();
			return temp;
		}
		/// <summary>Utility function to update the InternalData with mutex locking thread safety.</summary>
		/// <param name="state">InternalData obj to be copied to the internal one.</param>
		void UpdateState(const InternalData& state)
		{
			ScopedLockType tempLock(this->m_state_mutex);
			this->m_local_state = state;
		}
		/// <summary>Returns a copy of the internal InternalData obj with mutex locking thread safety.</summary>
		InternalData GetCurrentState()
		{
			ScopedLockType tempLock(this->m_state_mutex);
			return this->m_local_state;
		}
	};
}

#pragma once
#include <thread>
#include <mutex>
#include <functional>
#include <vector>

namespace sds
{
	/// <summary>
	/// Convenience class, contains using typedefs for first two args of the
	///	user-supplied lambda function.
	/// </summary>
	struct LambdaArgs
	{
		using LambdaArg1 = std::atomic<bool>;
		using LambdaArg2 = std::mutex;
	};
	/// <summary>
	///	Lambda function runner base, has start() stop() isrunning() member functions.
	///	Runs a lambda on it's own thread.
	///	Template type "InternalData" must be default constructable!
	///	Instantiation requires a lambda of a certain form: function{void(atomic{bool}&, mutex&, InternalData&)}
	/// </summary>
	template <class InternalData>
	class CPPLambdaBase
	{
	public:
		using LambdaType = std::function<void(std::atomic<bool>&, std::mutex&, InternalData&)>;
		using ScopedLockType = std::lock_guard<std::mutex>;

		CPPLambdaBase(LambdaType lambdaToRun) : m_lambda(std::move(lambdaToRun)) { }
		CPPLambdaBase(const CPPLambdaBase& other) = delete;
		CPPLambdaBase(CPPLambdaBase&& other) = delete;
		CPPLambdaBase& operator=(const CPPLambdaBase& other) = delete;
		CPPLambdaBase& operator=(CPPLambdaBase&& other) = delete;
		~CPPLambdaBase()
		{
			StopThread();
		}
	protected:
		LambdaType m_lambda;
		InternalData m_local_state{}; // default constructed type InternalData
		std::atomic<bool> m_is_stop_requested = false;
		std::unique_ptr<std::thread> m_local_thread;
		std::mutex m_state_mutex;
	public:
		/// <summary>
		/// Starts running a new thread for the lambda.
		/// </summary>
		///	<returns>true on success, false on failure.</returns>
		bool StartThread() noexcept
		{
			if (m_local_thread != nullptr)
			{
				return false;
			}
			m_is_stop_requested = false;
			m_local_thread = std::make_unique<std::thread>(m_lambda, std::ref(m_is_stop_requested), std::ref(m_state_mutex), std::ref(m_local_state));
			return m_local_thread->joinable();
		}
		bool IsRunning() const noexcept
		{
			if (m_local_thread != nullptr)
			{
				return m_local_thread->joinable();
			}
			return false;
		}
		/// <summary>
		/// Non-blocking way to stop a running thread.
		/// </summary>
		void RequestStop() noexcept
		{
			if (this->m_local_thread != nullptr)
			{
				this->m_is_stop_requested = true;
			}
		}
		/// <summary>
		/// Blocking way to stop a running thread, joins to current thread and waits.
		/// </summary>
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
				{
					//if it is not joinable, set to nullptr
					this->m_local_thread.reset();
				}
			}
		}
	};
}
#pragma once
#include <thread>
#include <mutex>
#include <functional>
namespace sds
{
	/// <summary>
	/// Base class for processing input concurrently, contains pure virtual member "workThread"
	/// to be overridden by an inheriting class. The startThread() and stopThread() protected members will allow
	/// control by the inheriting class. The std::mutex m_state_mutex and std::unique_ptr{std::thread} m_local_thread serve
	/// as concurrency helpers with std::lock_guard{std::mutex}
	/// Instantiate with the type you would like to have mutex protected access to within a running thread.
	/// </summary>
	template <class InternalData> class CPPThreadRunner
	{
	public:
		CPPThreadRunner() = default;
		CPPThreadRunner(const CPPThreadRunner& other) = delete;
		CPPThreadRunner(CPPThreadRunner&& other) = delete;
		CPPThreadRunner& operator=(const CPPThreadRunner& other) = delete;
		CPPThreadRunner& operator=(CPPThreadRunner&& other) = delete;
		/// <summary>
		/// Virtual destructor, the running thread should be stopped in the inherited class,
		/// before the member function "workThread" is destructed.
		/// </summary>
		virtual ~CPPThreadRunner() = default;
	private:
		std::unique_ptr<std::thread> m_local_thread;
	protected:
		//Interestingly, accessibility modifiers (public/private/etc.) work on "using" typedefs!
		using lock = std::lock_guard<std::mutex>;
		std::atomic<bool> m_is_thread_running = false;
		std::atomic<bool> m_is_stop_requested = false;
		InternalData m_local_state = {};
		std::mutex m_state_mutex;
		/// <summary>
		/// Pure virtual worker thread, intended to be overridden with something useful by an inheriting class.
		/// Protected visibility.
		/// </summary>
		virtual void workThread() = 0; //<<-- thread to be running.
		/// <summary>
		/// Starts running a new "workThread".
		/// </summary>
		void startThread()
		{
			if (!this->m_is_thread_running)
			{
				if (this->m_local_thread == nullptr)
				{
					this->m_is_stop_requested = false;
					this->m_is_thread_running = true;
					this->m_local_thread = std::make_unique<std::thread>([this] { workThread(); });
				}
				else
				{
					if (this->m_local_thread->joinable())
					{
						this->m_is_stop_requested = true;
						this->m_local_thread->join();
					}
					this->m_local_thread.reset(); //reset the shared_ptr (call's dtor, deletes object if unique)
					this->m_is_stop_requested = false;
					this->m_is_thread_running = true;
					this->m_local_thread = std::make_unique<std::thread>([this] { workThread(); });
				}
			}
		}
		/// <summary>
		/// Non-blocking way to stop a running thread.
		/// </summary>
		void requestStop()
		{
			if (this->m_local_thread != nullptr)
			{
				//if thread not running, return
				if (!this->m_is_thread_running)
				{
					return;
				}
				//else request thread stop
				else
				{
					this->m_is_stop_requested = true;
				}
			}
		}
		/// <summary>
		/// Blocking way to stop a running thread, joins to current thread and waits.
		/// </summary>
		void stopThread()
		{
			//Get this setting out of the way.
			this->m_is_stop_requested = true;
			//If there is a thread obj..
			if (this->m_local_thread != nullptr)
			{
				//if it is not joinable, set to nullptr
				if (!this->m_local_thread->joinable())
				{
					this->m_local_thread.reset();
					this->m_is_thread_running = false;
					return;
				}
				else
				{
					this->m_local_thread->join();
					this->m_local_thread.reset();
					this->m_is_thread_running = false;
				}
			}
		}
		/// <summary>
		/// Utility function to update the InternalData with mutex locking thread safety.
		/// </summary>
		/// <param name="state">InternalData obj to be copied to the internal one.</param>
		void updateState(const InternalData& state)
		{
			lock l1(m_state_mutex);
			m_local_state = state;
		}
		/// <summary>
		/// Returns a copy of the internal InternalData obj with mutex locking thread safety.
		/// </summary>
		/// <returns>A copy of the internal InternalData obj</returns>
		InternalData getCurrentState()
		{
			lock l1(m_state_mutex);
			return m_local_state;
		}

	};
}
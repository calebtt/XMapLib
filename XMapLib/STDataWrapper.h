#pragma once
#include "stdafx.h"
#include <mutex>
#include <atomic>
#include <functional>

namespace sds
{
	/// <summary> <para><c>STDataWrapper</c> is a base class for function objects that
	///	manage their own access to the data they want protected while running
	///	on another thread. Operator() is overloaded in derived classes to encapsulate
	///	the code to be ran concurrently, and the user should provide concurrent access
	///	methods such as "GetCopyOfData()" or "ClearData()".</para>
	/// <para>Choosing this approach allows many of these function objects to be ran
	/// on a single extra thread, it is appropriate for polling functions that must be
	/// ran indefinitely with no loop delay, but do little more than make system calls
	/// and report the information to somewhere else.</para> </summary>
	class STDataWrapper
	{
	public: /* Giant list of using declaration and other aliases. */
		using MutType = std::mutex; // The type of the mutex used for general access coordination.
		using AtomicBool = std::atomic<bool>; // Alias for stop condition type, not pointer wrapped.
		using ScopedLockType = std::lock_guard<MutType>; // Alias for chosen scoped lock type.
		using LogFnType = std::function<void(std::string)>; // Alias for logging function pointer type.
	protected: /* Section for used data members */
		const LogFnType LogFn;
		MutType m_mutex;
		//A bool provided for user code use, to disable processing of the
		//operator()() function.
		AtomicBool m_is_enabled{ true };
		//An optional marker that can be toggled in the user class
		//to indicate if the function performed meaningful work. This
		//can be helpful for limiting resource utilization.
		AtomicBool m_did_work{ true };
	public:
		//Constructor, LogFnType is an optional logging function that accepts const char* as the only arg.
		explicit STDataWrapper(const LogFnType fn = nullptr) : LogFn(fn) { }
		// Pure virtual operator() overload, to be overridden in derived classes.
		virtual void operator()() = 0;
		virtual ~STDataWrapper() = default;
		virtual bool IsEnabled() const noexcept { return m_is_enabled; }
		virtual bool DidMeaningfulWork() const noexcept { return m_did_work; }
	};
}
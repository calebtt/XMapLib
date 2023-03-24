#pragma once
#include "pch.h"

namespace sds::Utilities
{
	namespace ThreadPrior
	{
		// Functions that wrap the platform dependent process of setting the thread priority.
		// Provides a simplistic interface for performing thread priority changes.
		// This header interface will have the implementation file changed per platform, when multiplatform support is finished.
		inline
		bool SetPriorityHigh()
		{
			const HANDLE currentThreadHandle = GetCurrentThread();
			return SetThreadPriority(currentThreadHandle, THREAD_PRIORITY_HIGHEST) != 0;
		}
		inline
		bool SetPriorityLow()
		{
			const HANDLE currentThreadHandle = GetCurrentThread();
			return SetThreadPriority(currentThreadHandle, THREAD_PRIORITY_LOWEST) != 0;
		}
		inline
		bool SetPriorityNormal()
		{
			const HANDLE currentThreadHandle = GetCurrentThread();
			return SetThreadPriority(currentThreadHandle, THREAD_PRIORITY_NORMAL) != 0;
		}
	}
}

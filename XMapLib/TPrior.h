#pragma once
#include "stdafx.h"

namespace sds::Utilities
{
	/// <summary> Wraps the platform dependent process of setting the thread priority.
	/// Provides a simplistic interface for performing thread priority changes.
	/// This header interface will have the implementation file changed per platform, when multiplatform support is finished.
	///	</summary>
	class TPrior
	{
	public:
		bool SetPriorityHigh()
		{
			HANDLE currentThreadHandle = GetCurrentThread();
			return SetThreadPriority(currentThreadHandle, THREAD_PRIORITY_HIGHEST) != 0;
		}
		bool SetPriorityLow()
		{
			HANDLE currentThreadHandle = GetCurrentThread();
			return SetThreadPriority(currentThreadHandle, THREAD_PRIORITY_LOWEST) != 0;
		}
		bool SetPriorityNormal()
		{
			HANDLE currentThreadHandle = GetCurrentThread();
			return SetThreadPriority(currentThreadHandle, THREAD_PRIORITY_NORMAL) != 0;
		}
	};
}

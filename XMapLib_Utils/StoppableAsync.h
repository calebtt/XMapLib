#pragma once
#include "pch.h"
#include <thread>
#include <memory>
#include <future>
#include <functional>

namespace sds::AsyncUtil
{
	class AsyncStopper
	{
		std::shared_ptr<std::atomic<bool>> stopValue{ std::make_shared<std::atomic<bool>>(false) };
	public:
		[[nodiscard]]
		auto IsStopRequested() const noexcept -> bool
		{
			return *stopValue;
		}
		auto SetStopValue(const bool isRequested) const noexcept
		{
			*stopValue = isRequested;
		}
	};

	struct PausableAsync
	{
		AsyncStopper stopper;
		std::shared_future<void> taskFuture;
	};

	/// <summary>
	///	Wraps a lambda with arguments, but no return value, into an argument-less std::function.
	///	The function also applies the AsyncStopRequested stop token logic, it will return immediately
	///	and perform no action if stop has been requested. The check is performed before the task is executed.
	///	</summary>
	/// <typeparam name="F"> The type of the function. </typeparam>
	/// <typeparam name="A"> The types of the arguments. </typeparam>
	///	<param name="stopToken"> Used to cancel processing. </param>
	/// <param name="taskFn"> The function to push. </param>
	/// <param name="args"> The arguments to pass to the function (by value). </param>
	template <typename F, typename... A>
	auto make_pausable_task(const AsyncStopper& stopToken, const F& taskFn, const A&... args)
	{
		if constexpr (sizeof...(args) == 0)
		{
			return std::function<void()>{[stopToken, taskFn]()
				{
					if (stopToken.IsStopRequested())
						return;
					taskFn();
			} };
		}
		else
		{
			return std::function<void()>{[stopToken, taskFn, args...]()
				{
					if (stopToken.IsStopRequested())
						return;
					taskFn(args...);
			} };
		}
	}

	/// <summary>
	///	Takes a range of pause-able tasks and returns a single std::function that calls them
	///	in succession. The range of functions is copied into the lambda.
	/// </summary>
	template<class FnRange_t>
	auto make_async_runnable_package(const FnRange_t& taskList)
	{
		return std::function<void()>
		{[taskList]()
			{
				for (const auto& elem : taskList)
				{
					elem();
				}
		}
		};
	}

	/// <summary>
	/// Starts the task running on (presumably) another thread from the pool via std::async,
	///	returns a shared_future and a pointer to the stopper object.
	/// </summary>
	/// <param name="stopToken"></param>
	/// <param name="task"></param>
	/// <param name="launchPolicy"></param>
	/// <returns></returns>
	template<typename Fn_t>
	auto start_stoppable_async(const AsyncStopper& stopToken, const Fn_t& task, std::launch launchPolicy = std::launch::async)
		-> PausableAsync
	{
		return PausableAsync{ stopToken, std::async(launchPolicy, task).share() };
	}
}
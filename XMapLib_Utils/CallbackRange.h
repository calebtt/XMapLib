#pragma once
#include "pch.h"
#include <memory>
#include <functional>
#include <deque>
#include <ranges>

namespace sds
{
    /// <summary> Concept for a range of std::function or something convertible to it. </summary>
    template<typename FnRange_t>
    concept IsFnRange = requires(FnRange_t & t)
    {
        { std::ranges::range<FnRange_t> };
        { std::convertible_to<typename FnRange_t::value_type, std::function<void()>> };
    };

    /// <summary>
    /// CallbackRange provides a container that holds tasks, and some functions
    /// for operating on it.
    /// </summary>
    class CallbackRange
    {
    public:
        using TaskInfo = std::function<void()>;
    public:
        // Public data member, allows direct access to the task source.
        std::deque<TaskInfo> TaskList{};
    public:
        CallbackRange() = default;
        CallbackRange(const IsFnRange auto& taskList)
        {
            TaskList = taskList;
        }
        CallbackRange(const TaskInfo& ti)
        {
            PushInfiniteTaskBack(ti);
        }
    public:
        /// <summary> Calls operator() on the collection of tasks. </summary>
        void operator()() const
        {
            for (const auto& elem : TaskList)
                if(elem != nullptr)
                    elem();
        }

        [[nodiscard]]
        auto empty() const noexcept -> bool
        {
            return TaskList.empty();
        }
    public:
        /// <summary> Push a function with zero or more arguments, but no return value, into the task list. </summary>
        /// <typeparam name="F"> The type of the function. </typeparam>
        /// <typeparam name="A"> The types of the arguments. </typeparam>
        /// <param name="taskFn"> The function to push. </param>
        /// <param name="args"> The arguments to pass to the function (by value). </param>
        template <typename F, typename... A>
        void PushInfiniteTaskBack(const F& taskFn, const A&... args)
        {
            if constexpr (sizeof...(args) == 0)
            {
                TaskList.emplace_back(TaskInfo{taskFn});
            }
            else
            {
                TaskList.emplace_back(TaskInfo([taskFn, args...] { taskFn(args...); }));
            }
        }

        /// <summary> Push a function with zero or more arguments, but no return value, into the task list. </summary>
        /// <typeparam name="F"> The type of the function. </typeparam>
        /// <typeparam name="A"> The types of the arguments. </typeparam>
        /// <param name="task"> The function to push. </param>
        /// <param name="args"> The arguments to pass to the function (by value). </param>
        template <typename F, typename... A>
        void PushInfiniteTaskFront(const F& task, const A&... args)
        {
            if constexpr (sizeof...(args) == 0)
            {
                TaskList.emplace_front(TaskInfo{task});
            }
            else
            {
                TaskList.emplace_front(TaskInfo([task, args...] { task(args...); }));
            }
        }

        /// <summary> Sets the underlying task list to the new range. </summary>
        /// <param name="taskContainer"> range meeting IsFnRange concept </param>
        void ResetTaskList(const IsFnRange auto& taskContainer)
        {
            TaskList = {};
            for (const auto& elem : taskContainer)
            {
                TaskList.emplace_back(elem);
            }
        }
    };

}
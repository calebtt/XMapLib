#pragma once
/*
 * Contains some free functions related to the tick and update loop.
 */
#include "stdafx.h"
#include <atomic>
#include <future>
#include <condition_variable>
#include "KeyboardMapSource.h"
#include "KeyboardPoller.h"
#include "KeyboardSettingsPack.h"

namespace sds
{
	/**
	 * \brief Process Fn gets an updated state from the poller, then processes it via
	 * mappings, and then notifies the completionNotifier that a new state is available for processing.
	 * \param mappings 
	 * \param poller 
	 * \param playerId 
	 * \param completionNotifier 
	 */
	inline
	auto ProcessFn(
		KeyboardMapSource& mappings, 
		KeyboardPoller& poller, 
		int playerId,
		std::atomic<bool>& completionNotifier)
	{
		mappings.ProcessState(poller.GetUpdatedState(playerId));
		completionNotifier = true;
		completionNotifier.notify_all();
	}

	/**
	 * \brief Tick Fn resets the completionNotifier, then adds a "ProcessFn" task to the async pool and waits for completion of processfn.
	 * \param stopReq 
	 * \param completionNotifier 
	 * \param processFn 
	 * \return 
	 */
	inline
	auto TickFn(
		std::atomic<bool>& stopReq,
		std::atomic<bool>& completionNotifier,
		std::function<void()> processFn)
	{
		while (!stopReq.stop_requested())
		{
			completionNotifier = false;
			//TODO if the notify/wait stuff isn't helpful, remove it.
			std::async(processFn);
			completionNotifier.wait(false);
		}
	}
}
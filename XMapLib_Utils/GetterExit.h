#pragma once
#include "pch.h"

// used to asynchronously await the enter key being pressed, before
// dumping the contents of the key maps.
class GetterExit
{
	std::unique_ptr<std::jthread> workerThread{};
	std::atomic<bool> m_exitState{ false };
public:
	GetterExit() { startThread(); }
	~GetterExit() { stopThread(); }
	//Returns a bool indicating if the thread should stop.
	bool operator()() { return m_exitState; }
protected:
	void stopThread() const
	{
		if (workerThread)
			if (workerThread->joinable())
				workerThread->join();
	}
	void startThread()
	{
		workerThread = std::make_unique<std::jthread>([this]() { workThread(); });
	}
	void workThread()
	{
		std::cin.get(); // block and wait for enter key
		std::cin.clear();
		m_exitState = true;
	}
};
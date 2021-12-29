// XMapLib.cpp : Defines the entry point for the console application.
//Caleb Taylor
//Keep in mind need to run the .exe in administrator mode to work with programs running in admin mode.
#include "stdafx.h"
#include "Utilities.h"
#include "KeyboardMapper.h"
#include "MouseMapper.h"

using namespace std;
void AddTestKeyMappings(sds::KeyboardMapper& mapper);

class GetExit : public sds::CPPThreadRunner<int>
{
	std::atomic<bool> m_exitState = false;
	const sds::KeyboardMapper& m_mp;
public:
	GetExit(const sds::KeyboardMapper &m) : CPPThreadRunner<int>(), m_mp(m) { startThread(); }
	~GetExit() override { stopThread(); }
	//Returns a bool indicating if the thread should stop.
	bool operator()() {	return m_exitState; }
protected:
	void workThread() override
	{
		this->m_is_thread_running = true;
		std::cin.get(); // block and wait
		auto mapList = m_mp.GetMaps();
		std::for_each(begin(mapList), end(mapList), [](const sds::KeyboardKeyMap& theMap)
			{
				cout << theMap << endl;
			});
		m_exitState = true;
		this->m_is_thread_running = false;
	}
};
/* Entry Point */
int main()
{
	using namespace sds;
	using namespace sds::Utilities;
	MousePlayerInfo player;
	KeyboardPlayerInfo kplayer;
	MouseMapper mouser(player);
	KeyboardMapper keyer(kplayer);
	AddTestKeyMappings(keyer);
	GetExit getter(keyer);
	std::string err = mouser.SetSensitivity(75); // 75 out of 100
	Utilities::LogError(err); // won't do anything if the string is empty
	mouser.SetStick(StickMap::RIGHT_STICK);
	cout << "[Enter] to dump keymap contents and quit." << endl;
	cout << "Xbox controller polling started..." << endl;
	do
	{
		const bool isControllerConnected = mouser.IsControllerConnected() && keyer.IsControllerConnected();
		const bool isThreadRunning = mouser.IsRunning() && keyer.IsRunning();
		if (!isThreadRunning && isControllerConnected)
		{
			std::cout << "Controller reported as: " << "Connected." << std::endl;
			keyer.Start();
			mouser.Start();
		}
		if ((!isControllerConnected) && isThreadRunning)
		{
			std::cout << "Controller reported as: " << "Disconnected." << std::endl;
			keyer.Stop();
			mouser.Stop();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (!getter());
	return 0;
}
void AddTestKeyMappings(sds::KeyboardMapper& mapper)
{
	using namespace sds;
	const auto buttons =
	{
		//https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
		KeyboardKeyMap{VK_PAD_LTRIGGER, VK_RBUTTON, false}, // right click
		KeyboardKeyMap{VK_PAD_RTRIGGER, VK_LBUTTON, false}, // left click
		KeyboardKeyMap{VK_PAD_LTHUMB_UP, 0x57, true}, // 'w'
		KeyboardKeyMap{VK_PAD_LTHUMB_LEFT, 0x41, true}, // 'a'
		KeyboardKeyMap{VK_PAD_LTHUMB_DOWN, 0x53, true}, // 's'
		KeyboardKeyMap{VK_PAD_LTHUMB_RIGHT, 0x44, true}, // 'd'
		KeyboardKeyMap{VK_PAD_DPAD_DOWN, VK_DOWN, true}, // 'downarrow'
		KeyboardKeyMap{VK_PAD_DPAD_UP, VK_UP, true}, // 'uparrow'
		KeyboardKeyMap{VK_PAD_DPAD_LEFT, VK_LEFT, true}, // 'leftarrow'
		KeyboardKeyMap{VK_PAD_DPAD_RIGHT, VK_RIGHT, true}, // 'rightarrow'
		KeyboardKeyMap{VK_PAD_LTHUMB_UPLEFT, 0x57, true}, // 'w'
		KeyboardKeyMap{VK_PAD_LTHUMB_UPLEFT, 0x41, true}, // 'a'
		KeyboardKeyMap{VK_PAD_LTHUMB_UPRIGHT, 0x57, true}, // 'w'
		KeyboardKeyMap{VK_PAD_LTHUMB_UPRIGHT, 0x44, true}, // 'd'
		KeyboardKeyMap{VK_PAD_LTHUMB_DOWNLEFT, 0x53, true}, // 's'
		KeyboardKeyMap{VK_PAD_LTHUMB_DOWNLEFT, 0x41, true}, // 'a'
		KeyboardKeyMap{VK_PAD_LTHUMB_DOWNRIGHT, 0x53, true}, // 's'
		KeyboardKeyMap{VK_PAD_LTHUMB_DOWNRIGHT, 0x44, true}, // 'd'
		KeyboardKeyMap{VK_PAD_A, VK_SPACE, false}, // ' '
		KeyboardKeyMap{VK_PAD_B, 0x45, false}, // 'e'
		KeyboardKeyMap{VK_PAD_X, 0x52, false} // 'r'
	};
	std::string errorCondition;
	ranges::for_each(ranges::begin(buttons), ranges::end(buttons), [&mapper, &errorCondition](const KeyboardKeyMap& m)
		{
			if (errorCondition.empty())
			{
				//cout << "Adding:\n" << m << endl;
				errorCondition = mapper.AddMap(m);
			}
		});
	if (!errorCondition.empty())
		cout << "Added buttons until error: " << errorCondition << endl;
	else
		cout << "Added: " << buttons.size() << " key mappings." << endl;
}
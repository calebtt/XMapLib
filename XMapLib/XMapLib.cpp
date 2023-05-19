// XMapLib.cpp : Defines the entry point for the console application.
//Caleb Taylor
//Keep in mind need to run the .exe in administrator mode to work with programs running in admin mode.
#include "stdafx.h"
#include "../XMapLib_Keyboard/KeyboardPollerController.h"
#include "../XMapLib_Keyboard/KeyboardActionTranslator.h"

using namespace std;

int main()
{
	using namespace sds;
	using namespace sds::Utilities;
	//construct some mapping objects...


	//auto IsControllerConnected = [](const int pid)
	//{
	//	return ControllerStatus::IsControllerConnected(pid);
	//};
	//ss << "[Enter] to dump keymap contents and quit." << endl;
	//ss << "Xbox controller polling started..." << endl;
	//ss << "Controller reported as: " << (IsControllerConnected(0) ? "Connected." : "Disconnected.") << std::endl;
	//ss.emit();

	//// main loop
	//do
	//{
	//	const bool isControllerConnected = ControllerStatus::IsControllerConnected(0);
	//	const bool isThreadRunning = mouser->IsRunning() && keyer->IsRunning();
	//	const bool doConnectStart = !isThreadRunning && isControllerConnected;
	//	const bool doConnectStop = isThreadRunning && !isControllerConnected;
	//	if (doConnectStart)
	//	{
	//		ss << "Controller reported as: Connected. Starting mapping objects." << std::endl;
	//		keyer->Start();
	//		mouser->Start();
	//		ss.emit();
	//	}
	//	if (doConnectStop)
	//	{
	//		ss << "Controller reported as: Disconnected. Stopping mapping objects." << std::endl;
	//		keyer->Stop();
	//		mouser->Stop();
	//		ss.emit();
	//	}
	//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//} while (!getter());
	//return 0;
}

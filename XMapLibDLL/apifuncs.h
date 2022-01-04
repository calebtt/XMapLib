#pragma once
#include "../XMapLib/MouseMapper.h"
#include "../XMapLib/KeyboardMapper.h"

extern "C"
{
	namespace StaticInstance
	{
		sds::KeyboardMapper kbd;
		sds::MouseMapper mmp;
	}
	__declspec(dllexport) inline void XMapLibInitBoth()
	{
		StaticInstance::kbd.Start();
		StaticInstance::mmp.Start();
	}
	__declspec(dllexport) inline void XMapLibInitMouse()
	{
		StaticInstance::mmp.Start();
	}
	__declspec(dllexport) inline void XMapLibStopMouse()
	{
		StaticInstance::mmp.Stop();
	}
	__declspec(dllexport) inline void XMapLibInitKeyboard()
	{
		StaticInstance::kbd.Start();
	}
	__declspec(dllexport) inline void XMapLibStopKeyboard()
	{
		StaticInstance::kbd.Stop();
	}
	__declspec(dllexport) inline void XMapLibStopBoth()
	{
		StaticInstance::kbd.Stop();
		StaticInstance::mmp.Stop();
	}
	__declspec(dllexport) inline bool XMapLibAddMap(int vkSender, int vkMapping, bool bUsesRepeat)
	{
		return StaticInstance::kbd.AddMap(sds::KeyboardKeyMap(vkSender, vkMapping, bUsesRepeat)).empty();
	}
	__declspec(dllexport) inline void XMapLibClearMaps()
	{
		StaticInstance::kbd.ClearMaps();
	}
	__declspec(dllexport) inline bool XMapLibIsControllerConnected()
	{
		return StaticInstance::kbd.IsControllerConnected() && StaticInstance::mmp.IsControllerConnected();
	}
	__declspec(dllexport) inline bool XMapLibIsMouseRunning()
	{
		return StaticInstance::mmp.IsRunning();
	}
	__declspec(dllexport) inline bool XMapLibIsKeyboardRunning()
	{
		return StaticInstance::kbd.IsRunning();
	}
	__declspec(dllexport) inline void XMapLibSetMouseStick(int whichStick)
	{
		switch(whichStick)
		{
		case 0:
			StaticInstance::mmp.SetStick(sds::StickMap::NEITHER_STICK);
			break;
		case 1:
			StaticInstance::mmp.SetStick(sds::StickMap::RIGHT_STICK);
			break;
		case 2:
			StaticInstance::mmp.SetStick(sds::StickMap::LEFT_STICK);
			break;
		default:
			break;
		}
	}
	__declspec(dllexport) inline bool XMapLibSetMouseSensitivity(int sens)
	{
		return StaticInstance::mmp.SetSensitivity(sens).empty();
	}
}
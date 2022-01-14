#pragma once
#include "../XMapLib/MouseMapper.h"
#include "../XMapLib/KeyboardMapper.h"
#include "helperfuncs.h"
extern "C"
{
	namespace StaticInstance
	{
		inline sds::KeyboardMapper kbd;
		inline sds::MouseMapper mmp;
		inline std::string mapInfoFormatted;
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
	__declspec(dllexport) inline const char * XMapLibGetMaps()
	{
		std::vector<sds::KeyboardKeyMap> maps = StaticInstance::kbd.GetMaps();
		std::string localString;
		std::for_each(std::begin(maps), std::end(maps), [&localString](const auto& elem)
			{
				localString << elem;
			});
		StaticInstance::mapInfoFormatted = localString;
		return StaticInstance::mapInfoFormatted.data();
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
		//ensure same underlying integral type, and that sds::StickMap is still an enum
		using StickType = std::underlying_type<sds::StickMap>::type;
		static_assert(std::is_enum_v<sds::StickMap>, "ensure sds::StickMap is still an enum");
		static_assert(std::is_same_v<StickType, decltype(whichStick)>, "ensure interface type and enum underlying type are the same");
		//pass along the (possibly arbitrary) value to the MouseMapper.
		StaticInstance::mmp.SetStick(static_cast<sds::StickMap>(whichStick));
	}
	__declspec(dllexport) inline bool XMapLibSetMouseSensitivity(int sens)
	{
		return StaticInstance::mmp.SetSensitivity(sens).empty();
	}
	__declspec(dllexport) inline int XMapLibGetMouseSensitivity()
	{
		return StaticInstance::mmp.GetSensitivity();
	}

}
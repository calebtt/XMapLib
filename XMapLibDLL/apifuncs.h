#pragma once
#include "../XMapLib/MouseMapper.h"
#include "../XMapLib/KeyboardMapper.h"
#include "helperfuncs.h"
#include "StaticInstance.h"

#ifndef XMPLIB_EXPORT
#define XMPLIB_EXPORT __declspec(dllexport) inline
#endif

namespace Stins
{
	// Instance of the MapStuff struct that has data members
	// used in the course of managing access to the static instances
	// of KeyboardMapper and MouseMapper.
	inline static MapStuff currentInstance{};
}
extern "C"
{
	/// <summary> Called to init both the keyboard mapper and the mouse mapper, starts
	/// running the static thread if needed. </summary>
	XMPLIB_EXPORT void XMapLibInitBoth()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		//if static thread not running, start it
		if (!Stins::currentInstance.threadPoolPtr->IsRunning())
			Stins::currentInstance.threadPoolPtr->CreateThread();
		//start both keyboard and mouse mappers.
		Stins::currentInstance.KeyboardMapperInstance.Start();
		Stins::currentInstance.MouseMapperInstance.Start();
	}

	/// <summary> Called to initialize just the mouse mapper. Starts the static thread if needed. </summary>
	XMPLIB_EXPORT void XMapLibInitMouse()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		if (!Stins::currentInstance.threadPoolPtr->IsRunning())
			Stins::currentInstance.threadPoolPtr->CreateThread();
		Stins::currentInstance.MouseMapperInstance.Start();
	}

	/// <summary> Called to stop the running mouse mapper. This disables processing and the static thread
	/// if not being used by the keyboard mapper. </summary>
	XMPLIB_EXPORT void XMapLibStopMouse()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.MouseMapperInstance.Stop();
		if(!Stins::currentInstance.KeyboardMapperInstance.IsRunning())
			Stins::currentInstance.threadPoolPtr->CreateThread();
	}

	/// <summary> Called to initialize just the keyboard mapper. Starts the static thread if needed. </summary>
	XMPLIB_EXPORT void XMapLibInitKeyboard()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		if (!Stins::currentInstance.threadPoolPtr->IsRunning())
			Stins::currentInstance.threadPoolPtr->CreateThread();
		Stins::currentInstance.KeyboardMapperInstance.Start();
	}

	/// <summary> Called to stop the running keyboard mapper. This disables processing and the static thread
	/// if not being used by the mouse mapper. </summary>
	XMPLIB_EXPORT void XMapLibStopKeyboard()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.KeyboardMapperInstance.Stop();
		if(!Stins::currentInstance.MouseMapperInstance.IsRunning())
			Stins::currentInstance.threadPoolPtr->CreateThread();
	}

	/// <summary> Called to disable processing for both keyboard and mouse mappers, and the static thread. </summary>
	XMPLIB_EXPORT void XMapLibStopBoth()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.KeyboardMapperInstance.Stop();
		Stins::currentInstance.MouseMapperInstance.Stop();
		Stins::currentInstance.threadPoolPtr->CreateThread();
	}

	/// <summary> Called to add a <c>sds::KeyboardKeyMap</c> map information to the
	///	running <c>KeyboardMapper</c>. </summary>
	/// <param name="vkSender">controller button virtual keycode</param>
	/// <param name="vkMapping">keyboard or mouse button virtual keycode</param>
	/// <param name="bUsesRepeat">boolean denoting if the key-repeat behavior should be used</param>
	/// <returns>true on success, false on failure</returns>
	XMPLIB_EXPORT bool XMapLibAddMap(int vkSender, int vkMapping, bool bUsesRepeat)
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.KeyboardMapperInstance.AddMap(sds::KeyboardKeyMap(vkSender, vkMapping, bUsesRepeat)).empty();
	}

	/// <summary> Called to clear controller to keyboard key mappings. </summary>
	XMPLIB_EXPORT void XMapLibClearMaps()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.KeyboardMapperInstance.ClearMaps();
	}

	/// <summary> Returns a pointer to a static internal buffer used to store information about all of the loaded
	///	controller key to keyboard/mouse key mappings. <b>This memory is managed internally!</b> </summary>
	///	<remarks> Returned pointer to string is only guaranteed to be valid until the next call to
	///	<c>XMapLibAddMap()</c> or <c>XMapLibClearMaps()</c> or another call to <c>XMapLibGetMaps()</c></remarks>
	/// <returns>pointer to beginning of string buffer</returns>
	XMPLIB_EXPORT const char * XMapLibGetMaps()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		const std::vector<sds::KeyboardKeyMap> maps = Stins::currentInstance.KeyboardMapperInstance.GetMaps();
		std::string localString;
		for (const auto &lmp : maps)
			localString << lmp;
		// Check map string fits into static buffer.
		if (localString.size() >= Stins::currentInstance.mapInfoFormatted.size())
			return nullptr;
		// Copy to static buffer, return pointer to begin.
		std::ranges::copy(localString, Stins::currentInstance.mapInfoFormatted.data());
		return Stins::currentInstance.mapInfoFormatted.data();
	}

	/// <summary> Returns true if controller is connected. </summary>
	XMPLIB_EXPORT bool XMapLibIsControllerConnected()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		const bool kbdYes = Stins::currentInstance.KeyboardMapperInstance.IsControllerConnected();
		const bool mouseYes = Stins::currentInstance.MouseMapperInstance.IsControllerConnected();
		return kbdYes || mouseYes;
	}

	/// <summary> Called to query the status of the mouse mapper AND the thread pool running. </summary>
	/// <returns> Returns true if STRunner is running AND the MouseMapper is enabled. </returns>
	XMPLIB_EXPORT bool XMapLibIsMouseRunning()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.MouseMapperInstance.IsRunning();
	}

	/// <summary> Called to query the status of the keyboard mapper AND the thread pool running. </summary>
	///	<returns> Returns true if STRunner is running AND the KeyboardMapper is enabled. </returns>
	XMPLIB_EXPORT bool XMapLibIsKeyboardRunning()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.KeyboardMapperInstance.IsRunning();
	}

	/// <summary> Called to enable MouseMapper processing on the controller stick, options include left/right/neither.
	///	<para>NEITHER_STICK = 0,
	///	RIGHT_STICK = 1,
	///	LEFT_STICK = 2 </para>
	/// </summary>
	/// <param name="whichStick">parameter denoting which stick is to be used for processing</param>
	///	<remarks>if the integral value argument is not a valid <c>StickMap</c> enum value, the behavior is undefined.</remarks>
	XMPLIB_EXPORT void XMapLibSetMouseStick(int whichStick)
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		//ensure same underlying integral type, and that sds::StickMap is still an enum
		using StickType = std::underlying_type_t<sds::StickMap>;
		static_assert(std::is_enum_v<sds::StickMap>, "ensure sds::StickMap is still an enum");
		static_assert(std::is_same_v<StickType, decltype(whichStick)>, "ensure interface type and sds::StickMap enum underlying type are the same");
		//pass along the (possibly arbitrary) value to the MouseMapper.
		Stins::currentInstance.MouseMapperInstance.SetStick(static_cast<sds::StickMap>(whichStick));
	}
	XMPLIB_EXPORT bool XMapLibSetMouseSensitivity(int sens)
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.MouseMapperInstance.SetSensitivity(sens).empty();
	}
	XMPLIB_EXPORT int XMapLibGetMouseSensitivity()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.MouseMapperInstance.GetSensitivity();
	}

}

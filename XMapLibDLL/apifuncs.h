#pragma once
#include "../XMapLib/MouseMapper.h"
#include "../XMapLib/KeyboardMapper.h"
#include "helperfuncs.h"
#include "StaticInstance.h"
namespace Stins
{
	// Instance of the MapStuff struct that has data members
	// used in the course of managing access to the static instances
	// of KeyboardMapper and MouseMapper.
	inline static MapStuff currentInstance{};
}
extern "C"
{
	/// <summary> Called to init both the keyboard mapper and the mouse mapper. </summary>
	__declspec(dllexport) inline void XMapLibInitBoth()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.kbd.Start();
		Stins::currentInstance.mmp.Start();
	}

	/// <summary> Called to initialize just the mouse mapper. </summary>
	__declspec(dllexport) inline void XMapLibInitMouse()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.mmp.Start();
	}

	/// <summary> Called to stop the running mouse mapper. This disables processing,
	/// but does not stop the thread pool thread. </summary>
	__declspec(dllexport) inline void XMapLibStopMouse()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.mmp.Stop();
	}

	/// <summary> Called to initialize just the keyboard mapper. </summary>
	__declspec(dllexport) inline void XMapLibInitKeyboard()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.kbd.Start();
	}

	/// <summary> Called to stop the running keyboard mapper. This disables processing,
	///	but does not stop the thread pool thread. </summary>
	__declspec(dllexport) inline void XMapLibStopKeyboard()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.kbd.Stop();
	}

	/// <summary> Called to disable processing for both keyboard and mouse mappers.
	///	Does not stop the thread pool thread. </summary>
	__declspec(dllexport) inline void XMapLibStopBoth()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.kbd.Stop();
		Stins::currentInstance.mmp.Stop();
	}

	/// <summary> Called to add a <c>sds::KeyboardKeyMap</c> map information to the
	///	running <c>KeyboardMapper</c>. </summary>
	/// <param name="vkSender">controller button virtual keycode</param>
	/// <param name="vkMapping">keyboard or mouse button virtual keycode</param>
	/// <param name="bUsesRepeat">boolean denoting if the key-repeat behavior should be used</param>
	/// <returns>true on success, false on failure</returns>
	__declspec(dllexport) inline bool XMapLibAddMap(int vkSender, int vkMapping, bool bUsesRepeat)
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.kbd.AddMap(sds::KeyboardKeyMap(vkSender, vkMapping, bUsesRepeat)).empty();
	}

	/// <summary> Called to clear controller to keyboard key mappings. </summary>
	__declspec(dllexport) inline void XMapLibClearMaps()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		Stins::currentInstance.kbd.ClearMaps();
	}

	/// <summary> Returns a pointer to a static internal buffer used to store information about all of the loaded
	///	controller key to keyboard/mouse key mappings. <b>This memory is managed internally!</b> </summary>
	///	<remarks> Returned pointer to string is only guaranteed to be valid until the next call to
	///	<c>XMapLibAddMap()</c> or <c>XMapLibClearMaps()</c> or another call to <c>XMapLibGetMaps()</c></remarks>
	/// <returns>pointer to beginning of string buffer</returns>
	__declspec(dllexport) inline const char * XMapLibGetMaps()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		const std::vector<sds::KeyboardKeyMap> maps = Stins::currentInstance.kbd.GetMaps();
		std::string localString;
		for (const auto &lmp : maps)
			localString << lmp;
		Stins::currentInstance.mapInfoFormatted = localString;
		return Stins::currentInstance.mapInfoFormatted.data();
	}

	/// <summary> Returns true if controller is connected. </summary>
	__declspec(dllexport) inline bool XMapLibIsControllerConnected()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		const bool kbdYes = Stins::currentInstance.kbd.IsControllerConnected();
		const bool mouseYes = Stins::currentInstance.mmp.IsControllerConnected();
		return kbdYes || mouseYes;
	}

	/// <summary> Called to query the status of the mouse mapper AND the thread pool running. </summary>
	/// <returns> Returns true if STRunner is running AND the MouseMapper is enabled. </returns>
	__declspec(dllexport) inline bool XMapLibIsMouseRunning()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.mmp.IsRunning();
	}

	/// <summary> Called to query the status of the keyboard mapper AND the thread pool running. </summary>
	///	<returns> Returns true if STRunner is running AND the KeyboardMapper is enabled. </returns>
	__declspec(dllexport) inline bool XMapLibIsKeyboardRunning()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.kbd.IsRunning();
	}

	/// <summary> Called to enable MouseMapper processing on the controller stick, options include left/right/neither.
	///	<para>NEITHER_STICK = 0,
	///	RIGHT_STICK = 1,
	///	LEFT_STICK = 2 </para>
	/// </summary>
	/// <param name="whichStick">parameter denoting which stick is to be used for processing</param>
	///	<remarks>if the integral value argument is not a valid <c>StickMap</c> enum value, the behavior is undefined.</remarks>
	__declspec(dllexport) inline void XMapLibSetMouseStick(int whichStick)
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		//ensure same underlying integral type, and that sds::StickMap is still an enum
		using StickType = std::underlying_type_t<sds::StickMap>;
		static_assert(std::is_enum_v<sds::StickMap>, "ensure sds::StickMap is still an enum");
		static_assert(std::is_same_v<StickType, decltype(whichStick)>, "ensure interface type and sds::StickMap enum underlying type are the same");
		//pass along the (possibly arbitrary) value to the MouseMapper.
		Stins::currentInstance.mmp.SetStick(static_cast<sds::StickMap>(whichStick));
	}
	__declspec(dllexport) inline bool XMapLibSetMouseSensitivity(int sens)
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.mmp.SetSensitivity(sens).empty();
	}
	__declspec(dllexport) inline int XMapLibGetMouseSensitivity()
	{
		Stins::LockType tempLock(Stins::currentInstance.accessBlocker);
		return Stins::currentInstance.mmp.GetSensitivity();
	}

}

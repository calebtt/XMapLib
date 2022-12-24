#pragma once
#include "../XMapLib/MouseMapper.h"
#include "../XMapLib/KeyboardMapper.h"
#include "helperfuncs.h"

#ifndef XMPLIB_EXPORT
#define XMPLIB_EXPORT __declspec(dllexport) inline
#endif

//namespace Stins
//{
	using LockType = std::scoped_lock <std::mutex>;
	/// <summary>
	/// Struct with some data members used in the course of managing access to the
	///	static instances of <c>MouseMapper</c> and <c>KeyboardMapper</c>.
	/// </summary>
	struct StaticMapper
	{
		using ThreadUnit_t = imp::ThreadUnitPlusPlus;
	private:
		// Aprox. 32kb buffer for map string.
		static constexpr std::size_t MapInfoBufferSize{ 1'024ull * 32 };
		// Mutex for single instance enforcement
		inline static std::mutex m_instanceMutex;
		// Mapper settings objects.
		sds::MouseSettingsPack m_msp{};
		sds::KeyboardSettingsPack m_ksp{};
	public:
		// Normal use access blocker mutex.
		std::mutex accessBlocker;
		std::shared_ptr<ThreadUnit_t> m_ThreadPoolPtr{ std::make_shared<ThreadUnit_t>() };
		sds::KeyboardMapper m_KeyboardMapperInstance{ m_ThreadPoolPtr, m_ksp };
		sds::MouseMapper<> m_MouseMapperInstance{ m_ThreadPoolPtr, m_msp, &ErrorCallb };
		// Buffer large enough to hold any map string likely possible.
		std::array<char, MapInfoBufferSize> m_MapInfoFormatted{};
		// ctor
		StaticMapper()
		{
			m_instanceMutex.lock();
		}
		~StaticMapper()
		{
			m_instanceMutex.unlock();
		}

		static void ErrorCallb(const std::string msg) noexcept
		{
			MessageBoxA(nullptr, msg.c_str(), "", MB_OK);
		}
	};
	// Instance of the MapStuff struct that has data members
	// used in the course of managing access to the static instances
	// of KeyboardMapper and MouseMapper.
	inline std::unique_ptr<StaticMapper> currentInstance;
//}
extern "C"
{
	/// <summary> Called to init both the keyboard mapper and the mouse mapper, starts
	/// running the static thread if needed. </summary>
	XMPLIB_EXPORT void XMapLibInitBoth()
	{
		currentInstance = std::make_unique<StaticMapper>();
		LockType tempLock(currentInstance->accessBlocker);
		//if static thread not running, start it
		//if (!currentInstance->m_ThreadPoolPtr->IsRunning())
		//	currentInstance->m_ThreadPoolPtr->CreateThread();
		//start both keyboard and mouse mappers.
		//currentInstance->m_KeyboardMapperInstance.Start();
		//currentInstance->m_MouseMapperInstance.Start();
	}

	/// <summary> Called to disable processing for both keyboard and mouse mappers, and the static thread. </summary>
	XMPLIB_EXPORT void XMapLibStopBoth()
	{
		LockType tempLock(currentInstance->accessBlocker);
		currentInstance->m_KeyboardMapperInstance.Stop();
		currentInstance->m_MouseMapperInstance.Stop();
		//currentInstance->m_ThreadPoolPtr->CreateThread();
	}

	/// <summary> Called to enable processing for both keyboard and mouse mappers, and the static thread. </summary>
	XMPLIB_EXPORT void XMapLibStartBoth()
	{
		LockType tempLock(currentInstance->accessBlocker);
		currentInstance->m_KeyboardMapperInstance.Start();
		currentInstance->m_MouseMapperInstance.Start();
		//currentInstance->m_ThreadPoolPtr->CreateThread();
	}

	/// <summary> Called to add a <c>sds::ControllerButtonToActionMap</c> map information to the
	///	running <c>KeyboardMapper</c>. </summary>
	/// <param name="vkSender">controller button virtual keycode</param>
	/// <param name="vkMapping">keyboard or mouse button virtual keycode</param>
	/// <param name="bUsesRepeat">boolean denoting if the key-repeat behavior should be used</param>
	/// <returns>true on success, false on failure</returns>
	XMPLIB_EXPORT bool XMapLibAddMap(int vkSender, int vkMapping, bool bUsesRepeat)
	{
		LockType tempLock(currentInstance->accessBlocker);
		return currentInstance->m_KeyboardMapperInstance.AddMap(sds::ControllerButtonToActionMap(vkSender, vkMapping, bUsesRepeat)).empty();
	}

	/// <summary> Called to clear controller to keyboard key mappings. </summary>
	XMPLIB_EXPORT void XMapLibClearMaps()
	{
		LockType tempLock(currentInstance->accessBlocker);
		currentInstance->m_KeyboardMapperInstance.ClearMaps();
	}

	/// <summary> Returns a pointer to a static internal buffer used to store information about all of the loaded
	///	controller key to keyboard/mouse key mappings. <b>This memory is managed internally!</b> </summary>
	///	<remarks> Returned pointer to string is only guaranteed to be valid until the next call to
	///	<c>XMapLibAddMap()</c> or <c>XMapLibClearMaps()</c> or another call to <c>XMapLibGetMaps()</c></remarks>
	/// <returns>pointer to beginning of string buffer</returns>
	XMPLIB_EXPORT const char * XMapLibGetMaps()
	{
		LockType tempLock(currentInstance->accessBlocker);
		const std::vector<sds::ControllerButtonToActionMap> maps = currentInstance->m_KeyboardMapperInstance.GetMaps();
		std::string localString;
		for (const auto &lmp : maps)
			localString << lmp;
		// Check map string fits into static buffer.
		if (localString.size() >= currentInstance->m_MapInfoFormatted.size())
			return nullptr;
		// Copy to static buffer, return pointer to begin.
		std::ranges::copy(localString, currentInstance->m_MapInfoFormatted.data());
		return currentInstance->m_MapInfoFormatted.data();
	}

	/// <summary> Returns true if controller is connected. </summary>
	XMPLIB_EXPORT bool XMapLibIsControllerConnected()
	{
		LockType tempLock(currentInstance->accessBlocker);
		const bool kbdYes = currentInstance->m_KeyboardMapperInstance.IsControllerConnected();
		const bool mouseYes = currentInstance->m_MouseMapperInstance.IsControllerConnected();
		return kbdYes || mouseYes;
	}

	/// <summary> Called to query the status of the mouse mapper AND the thread pool running. </summary>
	/// <returns> Returns true if STRunner is running AND the MouseMapper is enabled. </returns>
	XMPLIB_EXPORT bool XMapLibIsMouseRunning()
	{
		LockType tempLock(currentInstance->accessBlocker);
		return currentInstance->m_MouseMapperInstance.IsRunning();
	}

	/// <summary> Called to query the status of the keyboard mapper AND the thread pool running. </summary>
	///	<returns> Returns true if STRunner is running AND the KeyboardMapper is enabled. </returns>
	XMPLIB_EXPORT bool XMapLibIsKeyboardRunning()
	{
		LockType tempLock(currentInstance->accessBlocker);
		return currentInstance->m_KeyboardMapperInstance.IsRunning();
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
		LockType tempLock(currentInstance->accessBlocker);
		//ensure same underlying integral type, and that sds::StickMap is still an enum
		using StickType = std::underlying_type_t<sds::StickMap>;
		static_assert(std::is_enum_v<sds::StickMap>, "ensure sds::StickMap is still an enum");
		static_assert(std::is_same_v<StickType, decltype(whichStick)>, "ensure interface type and sds::StickMap enum underlying type are the same");
		//pass along the (possibly arbitrary) value to the MouseMapper.
		currentInstance->m_MouseMapperInstance.SetStick(static_cast<sds::StickMap>(whichStick));
	}
	XMPLIB_EXPORT bool XMapLibSetMouseSensitivity(int sens)
	{
		LockType tempLock(currentInstance->accessBlocker);
		return currentInstance->m_MouseMapperInstance.SetSensitivity(sens).empty();
	}
	XMPLIB_EXPORT int XMapLibGetMouseSensitivity()
	{
		LockType tempLock(currentInstance->accessBlocker);
		return currentInstance->m_MouseMapperInstance.GetSensitivity();
	}

}

#pragma once
#include "stdafx.h"
#include <bitset>
#include "XELog.h"
#include "MapFunctions.h"

namespace sds::Utilities
{
	/// <summary>
	/// Utility class for simulating input via Windows API.
	/// SendKeyInput is used primarily for simulating keyboard input.
	/// NOTE: Some applications, namely specific games, do not recognize input sent via virtual keycode and
	/// instead only register hardware scancodes.
	/// </summary>
	class SendKeyInput
	{
		bool m_auto_disable_numlock = true; // toggle this to make the default behavior not toggle off numlock on your keyboard
		std::map<int,int> m_scancode_store;
	public:
		/// <summary>
		/// Default Constructor
		/// </summary>
		SendKeyInput() = default;
		//Turn auto disable numlock on or off
		SendKeyInput(const bool autoDisable) : m_auto_disable_numlock(autoDisable) { }
		SendKeyInput(const SendKeyInput& other) = delete;
		SendKeyInput(SendKeyInput&& other) = delete;
		SendKeyInput& operator=(const SendKeyInput& other) = delete;
		SendKeyInput& operator=(SendKeyInput&& other) = delete;
		~SendKeyInput() = default;
		/// <summary>
		/// Sends the virtual keycode as a hardware scancode.
		/// </summary>
		/// <param name="vk"> is the Virtual Keycode of the keystroke you wish to emulate </param>
		/// <param name="down"> is a boolean denoting if the keypress event is KEYDOWN or KEYUP</param>
		void SendScanCode(const int vk, const bool down)
		{
			if (m_auto_disable_numlock)
			{
				const SHORT NumLockState = GetKeyState(VK_NUMLOCK);
				std::bitset<sizeof(SHORT)> bits(NumLockState);
				//if the low order bit is 1, it is toggled in the ON position
				if (bits[0])
				{
					//XELog::LogError("Numlock reported toggled.");
					INPUT numlockInput = {};
					numlockInput.type = INPUT_KEYBOARD;
					numlockInput.ki.wVk = VK_NUMLOCK;
					numlockInput.ki.dwExtraInfo = GetMessageExtraInfo();
					CallSendInput(&numlockInput, 1);
					numlockInput.type = INPUT_KEYBOARD;
					numlockInput.ki.dwFlags = KEYEVENTF_KEYUP;
					numlockInput.ki.wVk = VK_NUMLOCK;
					numlockInput.ki.dwExtraInfo = GetMessageExtraInfo();
					CallSendInput(&numlockInput, 1);
				}
			}
			INPUT tempInput = {};
			auto MakeItMouse = [this, &tempInput](const DWORD flagsDown, const DWORD flagsUp, const bool isDown)
			{
				tempInput.type = INPUT_MOUSE;
				if (isDown)
					tempInput.mi.dwFlags = flagsDown;
				else
					tempInput.mi.dwFlags = flagsUp;
				tempInput.mi.dwExtraInfo = GetMessageExtraInfo();
				CallSendInput(&tempInput, 1);
			};
			const WORD scanCode = GetScanCode(vk);
			if (scanCode == 0)
			{
				//try mouse
				switch (vk)
				{
				case VK_LBUTTON:
					MakeItMouse(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, down);
					break;
				case VK_RBUTTON:
					MakeItMouse(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP, down);
					break;
				case VK_MBUTTON:
					MakeItMouse(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP, down);
					break;
				case VK_XBUTTON1:
					[[fallthrough]];
				case VK_XBUTTON2:
					MakeItMouse(MOUSEEVENTF_XDOWN, MOUSEEVENTF_XUP, down);
					break;
				default:
					break;
				}
			}
			else
			{
				//do scancode
				tempInput = {};
				tempInput.type = INPUT_KEYBOARD;
				if (down)
					tempInput.ki.dwFlags = KEYEVENTF_SCANCODE;
				else
					tempInput.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
				tempInput.ki.wScan = scanCode;
				UINT ret = CallSendInput(&tempInput, 1);
				if (ret == 0)
					Utilities::LogError("SendInput returned 0");
			}
		}
		/// <summary>
		/// Utility function to map a Virtual Keycode to a scancode
		/// </summary>
		/// <param name="vk"> integer virtual keycode</param>
		/// <returns></returns>
		WORD GetScanCode(const int vk)
		{
			if (vk > std::numeric_limits<unsigned char>::max() || vk < std::numeric_limits<unsigned char>::min())
				return 0;
			int ret = 0;
			if(MapFunctions::IsInMap(vk, m_scancode_store, ret))
			{
				return static_cast<WORD>(ret);
			}
			else
			{
				const WORD scan = static_cast<WORD> (MapVirtualKeyExA(vk, MAPVK_VK_TO_VSC, 0));
				m_scancode_store[vk] = scan;
				return scan;
			}
		}
		/// <summary>
		/// One member function calls SendInput with the eventual built INPUT struct.
		///	This is useful for debugging or re-routing the output for logging/testing of a real-time system.
		/// </summary>
		/// <param name="inp">Pointer to first element of INPUT array.</param>
		/// <param name="numSent">Number of elements in the array to send.</param>
		UINT CallSendInput(INPUT* inp, size_t numSent) const
		{
			return SendInput(static_cast<UINT>(numSent), inp, sizeof(INPUT));
		}
	};
}
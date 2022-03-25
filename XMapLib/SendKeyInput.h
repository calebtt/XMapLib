#pragma once
#include "stdafx.h"
#include <bitset>
#include <climits>
#include "XELog.h"

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
		using ScanMapType = std::unordered_map<int, int>;
		using ScanCodeType = unsigned short;
		using VirtualKeyType = unsigned int;
		using PrintableType = char;
		using VkType = unsigned char;
		bool m_auto_disable_numlock{ true }; // toggle this to make the default behavior not toggle off numlock on your keyboard
		ScanMapType m_scancode_store{};
	public:
		/// <summary>Default Constructor</summary>
		SendKeyInput() = default;
		//Turn auto disable numlock on or off
		explicit SendKeyInput(const bool autoDisable) : m_auto_disable_numlock(autoDisable) { }
		SendKeyInput(const SendKeyInput& other) = delete;
		SendKeyInput(SendKeyInput&& other) = delete;
		SendKeyInput& operator=(const SendKeyInput& other) = delete;
		SendKeyInput& operator=(SendKeyInput&& other) = delete;
		~SendKeyInput() = default;
		/// <summary>Sends the virtual keycode as a hardware scancode.</summary>
		/// <param name="vk"> is the Virtual Keycode of the keystroke you wish to emulate </param>
		/// <param name="down"> is a boolean denoting if the keypress event is KEYDOWN or KEYUP</param>
		void SendScanCode(const int vk, const bool down) noexcept
		{
			if (m_auto_disable_numlock)
			{
				UnsetNumlockAsync();
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
				const UINT ret = CallSendInput(&tempInput, 1);
				if (ret == 0)
					Utilities::LogError("SendInput returned 0");
			}
		}
		/// <summary>Utility function to map a Virtual Keycode to a scancode</summary>
		/// <param name="vk"> integer virtual keycode</param>
		/// <returns></returns>
		[[nodiscard]] WORD GetScanCode(const int vk) noexcept
		{
			if (vk > std::numeric_limits<VkType>::max() || vk < std::numeric_limits<VkType>::min())
				return 0;
			if (m_scancode_store.contains(vk))
			{
				return static_cast<WORD>(m_scancode_store[vk]);
			}
			else
			{
				const WORD scan = static_cast<WORD> (MapVirtualKeyExA(vk, MAPVK_VK_TO_VSC, nullptr));
				m_scancode_store[vk] = scan;
				return scan;
			}
		}
		/// <summary>One member function calls SendInput with the eventual built INPUT struct.
		///	This is useful for debugging or re-routing the output for logging/testing of a real-time system.</summary>
		/// <param name="inp">Pointer to first element of INPUT array.</param>
		/// <param name="numSent">Number of elements in the array to send.</param>
		UINT CallSendInput(INPUT* inp, size_t numSent) const noexcept
		{
			return SendInput(static_cast<UINT>(numSent), inp, sizeof(INPUT));
		}
	private:
		void UnsetNumlockAsync() const noexcept
		{
			const ScanCodeType NumLockState = GetKeyState(static_cast<int>((VK_NUMLOCK)));
			const std::bitset<sizeof(ScanCodeType)* CHAR_BIT> bits(NumLockState);
			if (bits[0])
			{
				auto DoNumlockSend = [this]()
				{
					auto result = SendVirtualKey(VK_NUMLOCK, true, true);
					if (result != 1)
						Utilities::LogError("Error sending numlock keypress down.");
					std::this_thread::sleep_for(std::chrono::milliseconds(15));
					result = SendVirtualKey(VK_NUMLOCK, true, false);
					if (result != 1)
						Utilities::LogError("Error sending numlock keypress up.");
				};
				std::thread numLockSender(DoNumlockSend);
				numLockSender.detach(); // fire and forget
			}
		}
		/// <summary>Utility function to send a virtual keycode as input to the OS.
		/// Handles keyboard keys and several mouse click buttons.</summary>
		UINT SendVirtualKey(const VirtualKeyType vk, const bool isKeyboard, const bool sendDown) const noexcept
		{
			INPUT inp{};
			inp.type = isKeyboard ? INPUT_KEYBOARD : INPUT_MOUSE;
			if (isKeyboard)
			{
				inp.ki.dwFlags = sendDown ? 0 : KEYEVENTF_KEYUP;
				inp.ki.wVk = static_cast<WORD>(vk);
				inp.ki.dwExtraInfo = GetMessageExtraInfo();
			}
			else
			{
				switch (vk)
				{
				case VK_LBUTTON:
					inp.mi.dwFlags = sendDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
					break;
				case VK_RBUTTON:
					inp.mi.dwFlags = sendDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
					break;
				case VK_MBUTTON:
					inp.mi.dwFlags = sendDown ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
					break;
				case VK_XBUTTON1:
					[[fallthrough]]; //annotated fallthrough
				case VK_XBUTTON2:
					inp.mi.dwFlags = sendDown ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
					break;
				default:
					return 0;
				}
				inp.mi.dwExtraInfo = GetMessageExtraInfo();
			}
			return CallSendInput(&inp, 1);
		}
	};
}
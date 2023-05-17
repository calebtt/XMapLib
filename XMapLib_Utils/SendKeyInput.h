#pragma once
#include "pch.h"
#include <bitset>
#include <climits>
#include "XELog.h"
#include "IOFuncs.h"

namespace sds::Utilities
{
	/**
	 * \brief	One function calls SendInput with the eventual built INPUT struct.
	 *	This is useful for debugging or re-routing the output for logging/testing of a near-real-time system.
	 * \param inp	Pointer to first element of INPUT array.
	 * \param numSent	Number of elements in the array to send.
	 * \return	Returns number of input structures sent.
	 */
	inline
	auto CallSendInput(INPUT* inp, std::uint32_t numSent) noexcept
	-> UINT
	{
		return SendInput(static_cast<UINT>(numSent), inp, sizeof(INPUT));
	}

	/**
	 * \brief	Utility function to send a virtual keycode as input to the OS.
	 * \param vk	virtual keycode for key (not always the same as a hardware scan code!)
	 * \param isKeyboard	Is the source a keyboard or mouse?
	 * \param sendDown	Send key-down event?
	 * \return	Returns number of events sent.
	 * \remarks		Handles keyboard keys and several mouse click buttons.
	 */
	inline
	UINT SendVirtualKey(const auto vk, const bool isKeyboard, const bool sendDown) noexcept
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

	/**
	 * \brief	Function called to un-set "num lock" key asynchronously. It is a "fire and forget" operation
	 *	that spawns a thread and returns a shared_future to it. The thread attempts to unset the key, and will not re-attempt if it fails.
	 * \returns		The shared_future may contain an optional string error message.
	 * \remarks		Uses std::async()
	 */
	inline
	auto UnsetNumlockAsync() noexcept
	-> std::optional<std::shared_future<std::string>>
	{
		// Endian-ness of machine it's being compiled on.
		static constexpr bool IsLittlEnd{ std::endian::native == std::endian::little };

		const auto NumLockState = GetKeyState(static_cast<int>((VK_NUMLOCK)));
		const std::bitset<sizeof(NumLockState)* CHAR_BIT> bits(NumLockState);
		static_assert(bits.size() > 0);
		if (const bool IsNumLockSet = IsLittlEnd ? bits[0] : bits[bits.size() - 1])
		{
			auto DoNumlockSend = [&]() -> std::string
			{
				auto result = SendVirtualKey(VK_NUMLOCK, true, true);
				if (result != 1)
					return ("Error sending numlock keypress down.");
				std::this_thread::sleep_for(std::chrono::milliseconds(15));
				result = SendVirtualKey(VK_NUMLOCK, true, false);
				if (result != 1)
					return ("Error sending numlock keypress up.");
				return {};
			};
			std::shared_future<std::string> fut = std::async(std::launch::async, DoNumlockSend);
			return fut;
		}
		return {};
	}

	/**
	 * \brief	Utility function to map a Virtual Keycode to a scancode
	 * \param	vk integer virtual keycode
	 * \return	returns the hardware scancode of the key
	 */
	[[nodiscard]]
	inline
	auto GetScanCode(const int vk) noexcept -> std::optional<WORD>
	{
		using VirtKey_t = unsigned char;
		if (vk > std::numeric_limits<VirtKey_t>::max()
			|| vk < std::numeric_limits<VirtKey_t>::min())
			return {};
		const auto scan = static_cast<WORD> (MapVirtualKeyExA(vk, MAPVK_VK_TO_VSC, nullptr));
		return scan;
	}

	/**
	 * \brief	Calls UnsetNumlockAsync() and waits for <b>timeout</b> time for a completion result, if timeout then returns nothing.
	 *	If an error occurs within the timeout period, it logs the error.
	 * \param timeoutTime	Time in milliseconds to wait for a result from the asynchronous thread spawned to unset numlock.
	 * \remarks		Do not call this function in a loop, it has a synchronous wait time delay.
	 */
	inline
	void UnsetAndCheckNumlock(const std::chrono::milliseconds timeoutTime = std::chrono::milliseconds{ 20 })
	{
		// Calls the unset numlock async function, gets a shared_future from which to determine if it succeeded, within a reasonable timeout.
		const auto optFuture = UnsetNumlockAsync();
		// If the future returned indicates an action was performed (numlock was set, it spawned a thread)
		if (optFuture.has_value())
		{
			// Then we wait for completion for the timeout duration
			const auto waitResult = optFuture.value().wait_for(timeoutTime);
			// If we have a result in a reasonable timeframe...
			if (waitResult == std::future_status::ready)
			{
				// If not empty, it has an error msg
				if (!optFuture.value().get().empty())
				{
					LogError(optFuture.value().get());
				}
			}
		}
	}

	/**
	 * \brief	Utility class for simulating input via Windows API. SendKeyInput is used primarily for simulating keyboard input.
	 * <b>NOTE: Some applications, namely specific games, do not recognize input sent via virtual keycode and instead only register hardware scancodes.</b>
	 * \remarks		It caches the scancodes retrieved from the virtual keycode given to the function to reduce making WinAPI calls. It also caches the
	 *	INPUT structs that it builds with the relevant info.
	 */
	class SendKeyInput
	{
	private:
		using ScanMapType = std::unordered_map<int, int>;
		using VkUpDownPair = std::pair<int, bool>;
		using InputStructMapType = std::unordered_map<VkUpDownPair, INPUT, PairHasher>;
	private:
		static constexpr std::chrono::milliseconds NUMLOCK_WAIT{ std::chrono::milliseconds(20) };
		bool m_autoDisableNumlock{ true }; // toggle this to make the default behavior not toggle off numlock on your keyboard
		ScanMapType m_scancode_store{};
		InputStructMapType m_input_store{};
	public:
		SendKeyInput() = default;
		//Turn auto disable numlock on or off
		explicit SendKeyInput(const bool autoDisableNumLock) : m_autoDisableNumlock(autoDisableNumLock) { }
		SendKeyInput(const SendKeyInput& other) = default;
		SendKeyInput(SendKeyInput&& other) = default;
		SendKeyInput& operator=(const SendKeyInput& other) = default;
		SendKeyInput& operator=(SendKeyInput&& other) = default;
		~SendKeyInput() = default;

		/**
		 * \brief	Sends the virtual keycode as a hardware scancode
		 * \param virtualKeycode	is the Virtual Keycode of the keystroke you wish to emulate
		 * \param doKeyDown	is a boolean denoting if the keypress event is KEYDOWN or KEYUP
		 */
		void SendScanCode(const int virtualKeycode, const bool doKeyDown) noexcept
		{
			if (m_autoDisableNumlock)
			{
				m_autoDisableNumlock = false;
				UnsetAndCheckNumlock();
			}
			// Current int bool pair, used as the key for caching built INPUTs,
			// used in multiple places below.
			const auto currentPair = std::make_pair(virtualKeycode, doKeyDown);
			// Do test against cached values first, the unordered_map contains() lookup
			// is constant-time on average, with the worst case being linear.
			if (m_input_store.contains(currentPair))
			{
				m_input_store[currentPair].mi.dwExtraInfo = GetMessageExtraInfo();
				CallSendInput(&m_input_store[currentPair], 1);
				return;
			}
			// if not found in the cache, build a new one.
			INPUT tempInput = {};
			auto MakeItMouse = [&](const DWORD flagsDown, const DWORD flagsUp, const bool isDown)
			{
				tempInput.type = INPUT_MOUSE;
				if (isDown)
					tempInput.mi.dwFlags = flagsDown;
				else
					tempInput.mi.dwFlags = flagsUp;
				tempInput.mi.dwExtraInfo = GetMessageExtraInfo();
				//store the built INPUT
				m_input_store[currentPair] = tempInput;
				CallSendInput(&tempInput, 1);
			};
			const auto scanCode = GetScanCode(virtualKeycode);
			if (!scanCode)
			{
				//try mouse
				switch (virtualKeycode)
				{
				case VK_LBUTTON:
					MakeItMouse(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, doKeyDown);
					break;
				case VK_RBUTTON:
					MakeItMouse(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP, doKeyDown);
					break;
				case VK_MBUTTON:
					MakeItMouse(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP, doKeyDown);
					break;
				case VK_XBUTTON1:
					[[fallthrough]];
				case VK_XBUTTON2:
					MakeItMouse(MOUSEEVENTF_XDOWN, MOUSEEVENTF_XUP, doKeyDown);
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
				if (doKeyDown)
					tempInput.ki.dwFlags = KEYEVENTF_SCANCODE;
				else
					tempInput.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
				tempInput.ki.wScan = scanCode.value();
				//store the built INPUT in the cache
				m_input_store[currentPair] = tempInput;
				const UINT ret = CallSendInput(&tempInput, 1);
				if (ret == 0)
					LogError("SendInput returned 0");
			}
		}
	};

	// Compile-time asserts for the type above, copyable, moveable.
	static_assert(std::is_copy_constructible_v<SendKeyInput>);
	static_assert(std::is_copy_assignable_v<SendKeyInput>);
	static_assert(std::is_move_constructible_v<SendKeyInput>);
	static_assert(std::is_move_assignable_v<SendKeyInput>);

}
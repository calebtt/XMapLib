#pragma once
#include "stdafx.h"

namespace sds::Utilities
{
	/// <summary>
	/// Utility class for simulating mouse movement input via the Windows API.
	/// </summary>
	class SendMouseInput
	{
		INPUT m_mouse_move_input{};
	public:
		/// <summary>Default Constructor</summary>
		SendMouseInput()
		{
			m_mouse_move_input.type = INPUT_MOUSE;
			m_mouse_move_input.mi.dwFlags = MOUSEEVENTF_MOVE;
		}
		SendMouseInput(const SendMouseInput& other) = delete;
		SendMouseInput(SendMouseInput&& other) = delete;
		SendMouseInput& operator=(const SendMouseInput& other) = delete;
		SendMouseInput& operator=(SendMouseInput&& other) = delete;
		~SendMouseInput() = default;
		/// <summary>Sends mouse movement specified by X and Y number of pixels to move.</summary>
		///	<remarks>Cartesian coordinate plane, starting at 0,0</remarks>
		/// <param name="x">number of pixels in X</param>
		/// <param name="y">number of pixels in Y</param>
		void SendMouseMove(const int x, const int y)
		{
			m_mouse_move_input.mi.dx = static_cast<LONG>(x);
			m_mouse_move_input.mi.dy = static_cast<LONG>(y);
			m_mouse_move_input.mi.dwExtraInfo = GetMessageExtraInfo();
			//Finally, send the input
			CallSendInput(&m_mouse_move_input, 1);
		}
		/// <summary>One member function calls SendInput with the eventual built INPUT struct.
		/// This is useful for debugging or re-routing the output for logging/testing of a real-time system.</summary>
		/// <param name="inp">Pointer to first element of INPUT array.</param>
		/// <param name="numSent">Number of elements in the array to send.</param>
		static UINT CallSendInput(INPUT* inp, size_t numSent) noexcept
		{
			return SendInput(static_cast<UINT>(numSent), inp, sizeof(INPUT));
		}
	};
}
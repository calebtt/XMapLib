#pragma once
#include "pch.h"
#include "IOFuncs.h"

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
		SendMouseInput(const SendMouseInput& other) = default;
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
	};

}

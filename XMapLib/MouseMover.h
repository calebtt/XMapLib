#pragma once
#include "stdafx.h"
#include <cassert>
#include "Utilities.h"
#include "MouseMoveInfoPacket.h"
#include "MouseSettingsPack.h"

namespace sds
{
	/// <summary> <c>MouseMover</c> contains the microsecond delay mouse movement processing.
	///	<para>It uses <c>MouseMoveInfoPacket</c> structs to perform the appropriate mouse move based on the delays.</para>
	/// </summary>
	class MouseMover
	{
		const MouseSettingsPack m_msp;
		Utilities::SendMouseInput m_keySend{};
		Utilities::DelayManager xTime;
		Utilities::DelayManager yTime;
	public:
		explicit MouseMover(const MouseSettingsPack msp = {})
		: m_msp(msp),
		xTime( msp.settings.MICROSECONDS_MAX ),
		yTime( msp.settings.MICROSECONDS_MAX )
		{ }

		/// <summary> <c>PerformMove()</c> performs the microsecond delay mouse movement processing.
		///	<para>It uses <c>MouseMoveInfoPacket</c> structs to perform the appropriate mouse move based on the delays.</para> </summary>
		void PerformMove(const MouseMoveInfoPacket mmip)
		{
			using namespace std::chrono;
			//A loop with no delay, that checks each delay value
			//against a timepoint, and performs the move for that axis if it beyond the timepoint
			//and in that way, will perform the single pixel move with two different (non-blocking) variable time delays.
			const bool isXPos = mmip.is_x_positive;
			const bool isYPos = mmip.is_y_positive;
			const size_t xDelay = mmip.x_delay;
			const size_t yDelay = mmip.y_delay;
			// Assertion for delay values.
			assert(xDelay > 0 && yDelay > 0);
			const bool m_isXM = mmip.is_beyond_dz_x;
			const bool m_isYM = mmip.is_beyond_dz_y;

			const bool isXPast = xTime.IsElapsed();
			const bool isYPast = yTime.IsElapsed();
			if (m_isXM || m_isYM)
			{
				int xVal = 0;
				int yVal = 0;
				if (isXPast && m_isXM)
				{
					xVal = (isXPos ? m_msp.settings.PIXELS_MAGNITUDE : (-m_msp.settings.PIXELS_MAGNITUDE));
					xTime.Reset(xDelay);
				}
				if (isYPast && m_isYM)
				{
					//Y is inverted for conversion to screen coordinate plane, which has 0,0 at the top-left corner.
					yVal = (isYPos ? -m_msp.settings.PIXELS_MAGNITUDE : (m_msp.settings.PIXELS_MAGNITUDE)); // y is inverted
					yTime.Reset(yDelay);
				}
				m_keySend.SendMouseMove(xVal, yVal);
			}
		}
	};
}
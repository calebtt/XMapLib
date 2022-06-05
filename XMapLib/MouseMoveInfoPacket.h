#pragma once
#include "stdafx.h"
namespace sds
{
	///<summary> This is an information packet with delay values and other info constructed in
	/// STMouseMapping and sent for use in MouseMoveThread. </summary>
	///	<remarks>Delay values should never be negative or 0! <para>Axis positive-ness will already have
	///	any necessary negation done to them <strong>BEFORE</strong> they are added in this struct (for instance, xbox controller Y axis needs inverted).</para></remarks>
	struct MouseMoveInfoPacket
	{
		//delays init to 1, a zero or negative delay in the mouse move thread is bad.
		size_t x_delay{ 1 };
		size_t y_delay{ 1 };
		bool is_x_positive{true};
		bool is_y_positive{true};
		bool is_beyond_dz_x{false};
		bool is_beyond_dz_y{false};
	};
}
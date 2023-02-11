#pragma once
#include "LibIncludes.h"
namespace sds
{
	/**
	 * \brief This is an information packet with delay values and other info constructed in STMouseMapping and sent for use in MouseMoveThread.
	 * \remarks Delay values should never be negative or 0! Axis positive-ness will already have any necessary negation done to them BEFORE
	 * they are added in this struct (for instance, xbox controller Y axis needs inverted).
	 */
	struct MouseMoveInfoPacket
	{
		//delays init to 1, a zero or negative delay in the mouse move thread is bad.
		int x_delay{ 1 };
		int y_delay{ 1 };
		bool is_x_positive{true};
		bool is_y_positive{true};
		bool is_beyond_dz_x{false};
		bool is_beyond_dz_y{false};
	};
}
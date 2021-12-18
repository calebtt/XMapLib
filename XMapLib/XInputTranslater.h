/*
Responsible for producing an ActionDetails from
an XINPUT_STATE for consumption by the rest of the code.
*/
#pragma once
#include "stdafx.h"
#include "ButtonStateDown.h"

namespace sds
{
	/// <summary>
	/// Produces an sds::ActionDetails string from an XINPUT_STATE for consumption by the rest
	/// of the code. They are space delimited, the tokens are sds::MapInformation tokens.
	/// <example>
	/// This might look like: "X B LTRIGGER RTRIGGER LTHUMB:UP RTHUMB:DOWN"
	/// and another part of the code can use that information to simulate the input mapped to those.
	/// </example>
	/// </summary>
	class XInputTranslater
	{
		//Utility class with functions that test button/thumbstick/trigger for depressed or "down" status
		ButtonStateDown m_bsd;
	public:
		XInputTranslater() = default;
		XInputTranslater(const sds::PlayerInfo &player) : m_bsd(player)	{ }
		XInputTranslater(const XInputTranslater& other) = delete;
		XInputTranslater(XInputTranslater&& other) = delete;
		XInputTranslater& operator=(const XInputTranslater& other) = delete;
		XInputTranslater& operator=(XInputTranslater&& other) = delete;
		~XInputTranslater() = default;
		/// <summary>
		/// Produces an ActionDetails string from an XINPUT_STATE struct representing the current state
		/// of the controller, as in what buttons are depressed, what values the thumbsticks are at.
		/// </summary>
		/// <param name="state">state obj retrieved from XInputGetState()</param>
		/// <returns>ActionDetails string with the information of which buttons are depressed, 
		/// which thumbsticks and their direction values. Whitespace delimited.
		/// This might look like: "X B LTRIGGER RTRIGGER LTHUMB:UP RTHUMB:DOWN"</returns>
		[[nodiscard]] ActionDetails ProcessState(const XINPUT_STATE &state) const
		{
			ActionDetails details;
			//Buttons
			for(auto it = sds::sdsActionDescriptors.xin_buttons.cbegin(); it != sds::sdsActionDescriptors.xin_buttons.cend(); ++it)
			{
				if( m_bsd.ButtonDown(state,it->first ) ) 
				{
					details += it->first + sds::sdsActionDescriptors.delimiter;
				}
			}
			//Triggers
			if( m_bsd.TriggerDown(state,sds::sdsActionDescriptors.lTrigger) )
			{
				details += sds::sdsActionDescriptors.lTrigger + sds::sdsActionDescriptors.delimiter;
			}
			if( m_bsd.TriggerDown(state,sds::sdsActionDescriptors.rTrigger) )
			{
				details += sds::sdsActionDescriptors.rTrigger + sds::sdsActionDescriptors.delimiter;
			}
			//Thumbsticks
			std::string thumb = sds::sdsActionDescriptors.lThumb + sds::sdsActionDescriptors.moreInfo;
			//lThumb
			if( m_bsd.ThumbstickDown(state,thumb + sds::sdsActionDescriptors.up) )
			{
				details += thumb + sds::sdsActionDescriptors.up + sds::sdsActionDescriptors.delimiter;
			}
			if( m_bsd.ThumbstickDown(state,thumb + sds::sdsActionDescriptors.down) )
			{
				details += thumb + sds::sdsActionDescriptors.down + sds::sdsActionDescriptors.delimiter;
			}
			if( m_bsd.ThumbstickDown(state, thumb + sds::sdsActionDescriptors.left) )
			{
				details += thumb + sds::sdsActionDescriptors.left + sds::sdsActionDescriptors.delimiter;
			}
			if( m_bsd.ThumbstickDown(state, thumb + sds::sdsActionDescriptors.right) )
			{
				details += thumb + sds::sdsActionDescriptors.right + sds::sdsActionDescriptors.delimiter;
			}
			//rThumb
			thumb = sds::sdsActionDescriptors.rThumb + sds::sdsActionDescriptors.moreInfo;
			if( m_bsd.ThumbstickDown(state,thumb + sds::sdsActionDescriptors.up) )
			{
				details += thumb + sds::sdsActionDescriptors.up + sds::sdsActionDescriptors.delimiter;
			}
			if( m_bsd.ThumbstickDown(state,thumb + sds::sdsActionDescriptors.down) )
			{
				details += thumb + sds::sdsActionDescriptors.down + sds::sdsActionDescriptors.delimiter;
			}
			if( m_bsd.ThumbstickDown(state, thumb + sds::sdsActionDescriptors.left) )
			{
				details += thumb + sds::sdsActionDescriptors.left + sds::sdsActionDescriptors.delimiter;
			}
			if( m_bsd.ThumbstickDown(state, thumb + sds::sdsActionDescriptors.right) )
			{
				details += thumb + sds::sdsActionDescriptors.right + sds::sdsActionDescriptors.delimiter;
			}
			return details;
		}
	};

}
#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include "KeyboardKeyMap.h"

#include <iostream>
#include <chrono>


namespace sds
{
	/// <summary>
	/// Contains the logic for determining if a key press or mouse click should occur, uses sds::Utilities::SendKeyInput m_key_send to send the input.
	///	Function ProcessKeystroke(XINPUT_KEYSTROKE &stroke) is used to process a controller input structure.
	/// </summary>
	class KeyboardTranslator
	{
		const std::string ERR_BAD_VK = "Either WordData.MappedToVK OR WordData.SendElementVK is <= 0";
		const std::string ERR_DUP_KEYUP = "Sent a duplicate keyup event to handle thumbstick direction changing behavior.";
		using ClockType = std::chrono::high_resolution_clock;
		using PointInTime = std::chrono::time_point<ClockType>;
		using InpType = sds::KeyboardKeyMap::ActionType;
	private:
		Utilities::SendKeyInput m_key_send;
		std::vector<KeyboardKeyMap> m_map_token_info;
		KeyboardPlayerInfo m_local_player;
	public:
		explicit KeyboardTranslator(const KeyboardPlayerInfo &p) : m_local_player(p)
		{
		}
		KeyboardTranslator() = default;
		KeyboardTranslator(const KeyboardTranslator& other) = delete;
		KeyboardTranslator(KeyboardTranslator&& other) = delete;
		KeyboardTranslator& operator=(const KeyboardTranslator& other) = delete;
		KeyboardTranslator& operator=(KeyboardTranslator&& other) = delete;
		~KeyboardTranslator() = default;

		void ProcessKeystroke(const XINPUT_KEYSTROKE &stroke)
		{
			//Key update loop
			KeyUpdateLoop();
			//Key repeat loop
			KeyRepeatLoop();
			//search the map for a matching virtual key and send it
			for(auto &w: m_map_token_info)
			{
				if (w.SendingElementVK == stroke.VirtualKey)
				{
					this->Normal(w, stroke);
				}
			}
		}
		std::string AddKeyMap(KeyboardKeyMap w)
		{
			std::string result = CheckForVKError(w);
			if (!result.empty())
				return result;
			m_map_token_info.push_back(w);
			return "";
		}
		void ClearMap()
		{
			m_map_token_info.clear();
		}
		std::vector<KeyboardKeyMap> GetMaps() const
		{
			return m_map_token_info;
		}
	private:
		void KeyUpdateLoop()
		{
			//If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--
			//otherwise reset it immediately.
			for(auto &e: m_map_token_info)
			{
				const bool DoUpdate = (e.LastAction == InpType::KEYUP && e.LastSentTime.IsElapsed()) && e.UsesRepeat;
				const bool DoImmediate = e.LastAction == InpType::KEYUP && !e.UsesRepeat;
				if (DoUpdate || DoImmediate)
					e.LastAction = InpType::NONE;
			}
		}
		void KeyRepeatLoop()
		{
			for(auto &w: m_map_token_info)
			{
				using AT = sds::KeyboardKeyMap::ActionType;
				if (w.UsesRepeat && (((w.LastAction == AT::KEYDOWN) || (w.LastAction == AT::KEYREPEAT))))
				{
					if (w.LastSentTime.IsElapsed())
						this->SendTheKey(w, true, AT::KEYREPEAT);
				}
			}
		}
		/// <summary>
		/// Normal keypress simulation logic. 
		/// </summary>
		void Normal(KeyboardKeyMap &detail, const XINPUT_KEYSTROKE &stroke)
		{
			const bool DoDown = (detail.LastAction == InpType::NONE) && (stroke.Flags & static_cast<WORD>(InpType::KEYDOWN));
			const bool DoUp = ((detail.LastAction == InpType::KEYDOWN) || (detail.LastAction == InpType::KEYREPEAT)) && (stroke.Flags & static_cast<WORD>(InpType::KEYUP));
			if (DoDown)
			{
				KeyboardKeyMap overtaken;
				if (IsOvertaking(detail,overtaken))
					DoOvertaking(overtaken);
				else
					SendTheKey(detail, true, InpType::KEYDOWN);
			}
			else if (DoUp)
			{
				SendTheKey(detail, false, InpType::KEYUP);
			}
		}
		//Does the key send call, updates LastAction and updates LastSentTime
		void SendTheKey(KeyboardKeyMap& mp, const bool keyDown, KeyboardKeyMap::ActionType action)
		{
			//std::cerr << mp << std::endl; // temp logging
			mp.LastAction = action;
			m_key_send.SendScanCode(mp.MappedToVK, keyDown);
			mp.LastSentTime.Reset(KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT); // update last sent time
		}
		/// <summary>
		/// Check to see if a different axis of the same thumbstick has been pressed already
		/// </summary>
		/// <param name="detail">Newest element being set to keydown state</param>
		///	<param name="outOvertaken">out key set to the one being overtaken</param>
		/// <returns>true if is overtaking a thumbstick direction already depressed</returns>
		bool IsOvertaking(const KeyboardKeyMap &detail, KeyboardKeyMap &outOvertaken)
		{
			//Is detail a thumbstick direction map, and if so, which thumbstick.
			const auto leftAxisIterator = std::find(KeyboardSettings::THUMBSTICK_L_VK_LIST.begin(), KeyboardSettings::THUMBSTICK_L_VK_LIST.end(), detail.SendingElementVK);
			const auto rightAxisIterator = std::find(KeyboardSettings::THUMBSTICK_R_VK_LIST.begin(), KeyboardSettings::THUMBSTICK_R_VK_LIST.end(), detail.SendingElementVK);
			const bool leftStick = leftAxisIterator != KeyboardSettings::THUMBSTICK_L_VK_LIST.end();
			const bool rightStick = rightAxisIterator != KeyboardSettings::THUMBSTICK_R_VK_LIST.end();
			//find a key-down'd or repeat'd direction of the same thumbstick
			if (leftStick || rightStick)
			{
				//build list of key-down state maps that match the thumbstick of the current "detail" map.
				const auto stickSettingList = leftStick ? KeyboardSettings::THUMBSTICK_L_VK_LIST : KeyboardSettings::THUMBSTICK_R_VK_LIST;
				auto TestFunc = [&stickSettingList, &detail](const KeyboardKeyMap& elem)
				{
					if ((elem.LastAction == InpType::KEYDOWN || elem.LastAction == InpType::KEYREPEAT) && elem.SendingElementVK != detail.SendingElementVK)
						return std::ranges::find(stickSettingList, elem.SendingElementVK) != stickSettingList.end();
					return false;
				};
				const auto mpit = std::ranges::find_if(m_map_token_info, TestFunc);
				if (mpit == m_map_token_info.end())
					return false;
				outOvertaken = *mpit;
				return true;
			}
			return false;
		}
		void DoOvertaking(KeyboardKeyMap &detail)
		{
			SendTheKey(detail, false, InpType::KEYUP);
		}
		std::string CheckForVKError(const KeyboardKeyMap& detail) const
		{
			if ((detail.MappedToVK <= 0) || (detail.SendingElementVK <= 0))
			{
				std::stringstream error;
				error << detail;
				return std::string("Contents:\n") + error.str() + ERR_BAD_VK;
			}
			return "";
		}
	};

}
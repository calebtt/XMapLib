#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include "KeyboardKeyMap.h"

#include <iostream>
#include <iomanip>
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
		using ClockType = std::chrono::high_resolution_clock;
		using PointInTime = std::chrono::time_point<ClockType>;
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
			//TODO the thumbsticks can switch to a different direction without sending the "keyup" event at all,
			//so it needs to automatically send keyup when the thumbstick direction changes.
			std::ranges::for_each(m_map_token_info.begin(), m_map_token_info.end(), [this, &stroke](auto &w)
				{
					if (w.SendingElementVK == stroke.VirtualKey)
					{
						this->Normal(w, stroke);
					}
				});
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
	private:
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
		/// <summary>
		/// Normal keypress simulation logic. 
		/// </summary>
		void Normal(KeyboardKeyMap &detail, const XINPUT_KEYSTROKE &stroke)
		{
			using InpType = sds::KeyboardKeyMap::ActionType;
			//Does the key send call, updates LastAction and updates LastSentTime
			auto SendTheKey = [this](KeyboardKeyMap & mp, const bool keyDown, InpType action)
			{
				mp.LastAction = action;
				m_key_send.SendScanCode(mp.MappedToVK, keyDown);
				mp.LastSentTime.Reset(KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT); // update last sent time
			};
			const bool DoDown = (detail.LastAction == InpType::NONE) && (stroke.Flags & static_cast<WORD>(InpType::KEYDOWN));
			const bool DoUp = ((detail.LastAction == InpType::KEYDOWN) || (detail.LastAction == InpType::KEYREPEAT)) && (stroke.Flags & static_cast<WORD>(InpType::KEYUP));
			const bool DoRepeat = detail.UsesRepeat && ((detail.LastAction == InpType::KEYDOWN) || (detail.LastAction == InpType::KEYREPEAT)) && (stroke.Flags & static_cast<WORD>(InpType::KEYREPEAT));
			//If enough time has passed, reset a keyup to none to start the process again
			const bool DoUpdate = (detail.LastAction == InpType::KEYUP && detail.LastSentTime.IsElapsed());
			if (DoDown)
			{
				//quick check to see if a different thumbstick axis has been pressed already,
				//and to release it if the current press is a thumbstick
				const bool isLeftStick = IsInList(KeyboardSettings::THUMBSTICK_L_VK_LIST, detail.SendingElementVK);
				const bool isRightStick = IsInList(KeyboardSettings::THUMBSTICK_R_VK_LIST, detail.SendingElementVK);
				if(isLeftStick)
				{
					for (auto& elem : m_map_token_info)
					{
						if(IsInList(KeyboardSettings::THUMBSTICK_L_VK_LIST, elem.SendingElementVK) 
							&& (elem.LastAction == InpType::KEYDOWN || elem.LastAction == InpType::KEYREPEAT))
						{
							SendTheKey(elem, false, InpType::KEYUP);
							break;
						}
					}
				}
				if (isRightStick)
				{
					for (auto& elem : m_map_token_info)
					{
						if (IsInList(KeyboardSettings::THUMBSTICK_R_VK_LIST, elem.SendingElementVK)
							&& (elem.LastAction == InpType::KEYDOWN || elem.LastAction == InpType::KEYREPEAT))
						{
							SendTheKey(elem, false, InpType::KEYUP);
							break;
						}
					}
				}
				SendTheKey(detail, true, InpType::KEYDOWN);
			}
			else if (DoUp)
				SendTheKey(detail, false, InpType::KEYUP);
			else if (DoRepeat)
				SendTheKey(detail, true, InpType::KEYREPEAT);
			else if (DoUpdate)
				detail.LastAction = InpType::NONE;
		}
		template<typename ListType, typename ValueType>
		bool IsInList(const ListType &currentList, const ValueType currentValue) const
		{
			return std::ranges::find(currentList.begin(),
				currentList.end(),
				currentValue) != currentList.end();
		}
	};

}
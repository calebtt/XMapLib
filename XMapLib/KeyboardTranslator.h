#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include "KeyboardKeyMap.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ranges>

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
			const bool DoDown = (detail.LastAction == InpType::NONE) && (stroke.Flags & static_cast<WORD>(InpType::KEYDOWN));
			const bool DoUp = ((detail.LastAction == InpType::KEYDOWN) || (detail.LastAction == InpType::KEYREPEAT)) && (stroke.Flags & static_cast<WORD>(InpType::KEYUP));
			const bool DoRepeat = detail.UsesRepeat && ((detail.LastAction == InpType::KEYDOWN) || (detail.LastAction == InpType::KEYREPEAT)) && (stroke.Flags & static_cast<WORD>(InpType::KEYREPEAT));
			//If enough time has passed, reset a keyup to none to start the process again
			const bool DoUpdate = (detail.LastAction == InpType::KEYUP && detail.LastSentTime.IsElapsed());
			if (DoDown)
			{
				IsOvertaken(detail);
				SendTheKey(detail, true, InpType::KEYDOWN);
			}
			else if (DoUp)
				SendTheKey(detail, false, InpType::KEYUP);
			else if (DoRepeat)
				SendTheKey(detail, true, InpType::KEYREPEAT);
			else if (DoUpdate)
				detail.LastAction = InpType::NONE;
		}
		//Does the key send call, updates LastAction and updates LastSentTime
		void SendTheKey(KeyboardKeyMap& mp, const bool keyDown, KeyboardKeyMap::ActionType action)
		{
			mp.LastAction = action;
			m_key_send.SendScanCode(mp.MappedToVK, keyDown);
			mp.LastSentTime.Reset(KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT); // update last sent time
		};
		template<typename ListType, typename ValueType>
		bool IsInList(const ListType &currentList, const ValueType currentValue) const
		{
			return std::ranges::find(currentList.begin(),
				currentList.end(),
				currentValue) != currentList.end();
		}
		/// <summary>
		/// Check to see if a different axis of the same thumbstick has been pressed already
		/// </summary>
		/// <param name="detail">Newest element being set to keydown state</param>
		/// <returns>true if keyup sent</returns>
		bool IsOvertaken(const KeyboardKeyMap &detail)
		{
			using InpType = sds::KeyboardKeyMap::ActionType;
			//Determine if the current keydown'd KeyboardKeyMap is left or right thumbstick axis
			const bool leftAxis = IsInList(KeyboardSettings::THUMBSTICK_L_VK_LIST, detail.SendingElementVK);
			const bool rightAxis = IsInList(KeyboardSettings::THUMBSTICK_R_VK_LIST, detail.SendingElementVK);
			bool upSent = false;
			if (leftAxis || rightAxis)
			{
				//ranges is pretty cool
				auto allDownMaps = m_map_token_info | std::views::filter([](auto& m) { return m.LastAction == InpType::KEYDOWN || m.LastAction == InpType::KEYREPEAT; });
				std::ranges::for_each(allDownMaps.begin(), allDownMaps.end(), [&upSent, &leftAxis, this](auto& elem)
					{
						if (IsInList(leftAxis ? KeyboardSettings::THUMBSTICK_L_VK_LIST : KeyboardSettings::THUMBSTICK_R_VK_LIST, elem.SendingElementVK))
						{
							SendTheKey(elem, false, InpType::KEYUP);
							if (upSent)
								Utilities::LogError("Error in IsOvertaken(): " + ERR_DUP_KEYUP);
							upSent = true;
						}
					});
			}
			return upSent;
		}
	};

}
#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include <iostream>
#include <chrono>

namespace sds
{

	struct KeyboardApplyKeystroke
	{
		static auto ProcessKeystroke(
			const KeyStateWrapper& stroke,
			const ControllerButtonToActionMap& keyMap) noexcept
		{
			if (keyMap.SendingElementVK == stroke.VirtualKey)
			{
				//this->Normal(w, stroke);
			}
		}
		/// <summary> If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--
		///	otherwise reset it immediately. </summary>
		void KeyUpdateLoop()
		{
			//If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--
			//otherwise reset it immediately.
			for (auto& e : m_map_token_info)
			{
				const bool DoUpdate = (e.LastAction == InpType::KEYUP && e.LastSentTime.IsElapsed()) && e.UsesRepeat;
				const bool DoImmediate = e.LastAction == InpType::KEYUP && !e.UsesRepeat;
				if (DoUpdate || DoImmediate)
					e.LastAction = InpType::NONE;
			}
		}
		/// <summary> Checks each <c>ControllerButtonToActionMap</c> in <c>m_map_token_info</c>'s <c>LastSentTime</c> timer for being
		///	elapsed, and if so, sends the repeat keypress (if key repeat behavior is enabled for the map). </summary>
		void KeyRepeatLoop()
		{
			for (auto& w : m_map_token_info)
			{
				using AT = sds::ControllerButtonToActionMap::ActionType;
				if (w.UsesRepeat && (((w.LastAction == AT::KEYDOWN) || (w.LastAction == AT::KEYREPEAT))))
				{
					if (w.LastSentTime.IsElapsed())
						this->SendTheKey(w, true, AT::KEYREPEAT);
				}
			}
		}
		/// <summary>Normal keypress simulation logic.</summary>
		void Normal(ControllerButtonToActionMap& detail, const KeyStateWrapper& stroke)
		{
			const bool DoDown = (detail.LastAction == InpType::NONE) && (stroke.Flags & static_cast<WORD>(InpType::KEYDOWN));
			const bool DoUp = ((detail.LastAction == InpType::KEYDOWN) || (detail.LastAction == InpType::KEYREPEAT)) && (stroke.Flags & static_cast<WORD>(InpType::KEYUP));
			if (DoDown)
			{
				ControllerButtonToActionMap overtaken;
				if (IsOvertaking(detail, overtaken))
					DoOvertaking(overtaken);
				else
					SendTheKey(detail, true, InpType::KEYDOWN);
			}
			else if (DoUp)
			{
				SendTheKey(detail, false, InpType::KEYUP);
			}
		}
		/// <summary>Does the key send call, updates LastAction and updates LastSentTime</summary>
		void SendTheKey(ControllerButtonToActionMap& mp, const bool keyDown, ControllerButtonToActionMap::ActionType action) noexcept
		{
			mp.LastAction = action;
			m_key_send.SendScanCode(mp.MappedToVK, keyDown);
			mp.LastSentTime.Reset(m_ksp.settings.MICROSECONDS_DELAY_KEYREPEAT); // update last sent time
		}
		/// <summary>Check to see if a different axis of the same thumbstick has been pressed already</summary>
		/// <param name="detail">Newest element being set to keydown state</param>
		///	<param name="outOvertaken">out key set to the one being overtaken</param>
		/// <returns>true if is overtaking a thumbstick direction already depressed</returns>
		[[nodiscard]]
		auto IsOvertaking(const ControllerButtonToActionMap& detail, ControllerButtonToActionMap& outOvertaken) -> bool
		{
			//Is detail a thumbstick direction map, and if so, which thumbstick.
			const auto leftAxisIterator = std::ranges::find(m_ksp.settings.THUMBSTICK_L_VK_LIST, detail.SendingElementVK);
			const auto rightAxisIterator = std::ranges::find(m_ksp.settings.THUMBSTICK_R_VK_LIST, detail.SendingElementVK);
			const bool leftStick = leftAxisIterator != m_ksp.settings.THUMBSTICK_L_VK_LIST.end();
			const bool rightStick = rightAxisIterator != m_ksp.settings.THUMBSTICK_R_VK_LIST.end();
			//find a key-down'd or repeat'd direction of the same thumbstick
			if (leftStick || rightStick)
			{
				//build list of key-down state maps that match the thumbstick of the current "detail" map.
				const auto stickSettingList = leftStick ? m_ksp.settings.THUMBSTICK_L_VK_LIST : m_ksp.settings.THUMBSTICK_R_VK_LIST;
				auto TestFunc = [&stickSettingList, &detail](const ControllerButtonToActionMap& elem)
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
		/// <summary> Specific type of key send to send the input to handle the key-up that occurs
		///	when a thumbstick is rotated to the right angle to denote a new key. </summary>
		/// <param name="detail">Which key is being overtaken.</param>
		void DoOvertaking(ControllerButtonToActionMap& detail) noexcept
		{
			SendTheKey(detail, false, InpType::KEYUP);
		}
		/// <summary> Checks a <c>ControllerButtonToActionMap</c>'s <c>MappedToVK</c> and <c>SendingElementVK</c> members for out of bounds values. </summary>
		///	<returns> Error message on error, empty string otherwise. </returns>
		[[nodiscard]]
		auto CheckForVKError(const ControllerButtonToActionMap& detail) const -> std::string
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
	/// <summary>
	/// Contains the logic for determining if a key press or mouse click should occur, uses sds::Utilities::SendKeyInput m_key_send to send the input.
	///	Function ProcessKeystroke(KeyboardPoller::KeyStateWrapper &stroke) is used to process a controller input structure. Handles key-repeat behavior as well.
	/// </summary>
	///	<remarks>Intended to be used synchronously!</remarks>
	class KeyboardTranslator
	{
		// TODO separate the state machine logic from the activation comparison logic, and the activation operation logic.
		// Don't always need just a keyboard press or mouse click.
		using ClockType = std::chrono::steady_clock;
		using PointInTime = std::chrono::time_point<ClockType>;
		using InpType = sds::ControllerButtonToActionMap::ActionType;
		const std::string ERR_BAD_VK{ "Either WordData.MappedToVK OR WordData.SendElementVK is <= 0" };
		const std::string ERR_DUP_KEYUP{ "Sent a duplicate keyup event to handle thumbstick direction changing behavior." };
	private:
		Utilities::SendKeyInput m_key_send{};
		std::vector<ControllerButtonToActionMap> m_map_token_info{};
		KeyboardSettingsPack m_ksp;
	public:
		explicit KeyboardTranslator(const KeyboardSettingsPack &ksp = {})
		: m_ksp(ksp)
		{
		}
		KeyboardTranslator(const KeyboardTranslator& other) = delete;
		KeyboardTranslator(KeyboardTranslator&& other) = delete;
		KeyboardTranslator& operator=(const KeyboardTranslator& other) = delete;
		KeyboardTranslator& operator=(KeyboardTranslator&& other) = delete;
		/// <summary> Destructor cleans up in-progress key-presses before destruction. </summary>
		~KeyboardTranslator()
		{
			CleanupInProgressEvents();
		}
	public:
		/// <summary> Main function for use, processes <c>KeyboardPoller::KeyStateWrapper</c> into key presses and mouse clicks. </summary>
		/// <param name="stroke">A KeyStateWrapper containing controller button press information. </param>
		void ProcessKeystroke(const KeyStateWrapper &stroke)
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
		/// <summary>Call this function to send key-ups for any in-progress key presses.</summary>
		void CleanupInProgressEvents()
		{
			for(auto &m: m_map_token_info)
			{
				if(m.LastAction == InpType::KEYDOWN || m.LastAction == InpType::KEYREPEAT)
				{
					this->DoOvertaking(m);
				}
			}
		}

		// Functions that get "bound" to a callback which is activated. KeyboardMapSource handles the creation process.
		auto OnKeyDown(ControllerButtonToActionMap &keyInfo)
		{
			
		}
		auto OnKeyUp(ControllerButtonToActionMap &keyInfo)
		{

		}
		auto OnKeyRepeat(ControllerButtonToActionMap& keyInfo)
		{
			
		}
	private:
		/// <summary> If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--
		///	otherwise reset it immediately. </summary>
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
		/// <summary> Checks each <c>ControllerButtonToActionMap</c> in <c>m_map_token_info</c>'s <c>LastSentTime</c> timer for being
		///	elapsed, and if so, sends the repeat keypress (if key repeat behavior is enabled for the map). </summary>
		void KeyRepeatLoop()
		{
			for(auto &w: m_map_token_info)
			{
				using AT = sds::ControllerButtonToActionMap::ActionType;
				if (w.UsesRepeat && (((w.LastAction == AT::KEYDOWN) || (w.LastAction == AT::KEYREPEAT))))
				{
					if (w.LastSentTime.IsElapsed())
						this->SendTheKey(w, true, AT::KEYREPEAT);
				}
			}
		}
		/// <summary>Normal keypress simulation logic.</summary>
		void Normal(ControllerButtonToActionMap &detail, const KeyStateWrapper &stroke)
		{
			const bool DoDown = (detail.LastAction == InpType::NONE) && (stroke.Flags & static_cast<WORD>(InpType::KEYDOWN));
			const bool DoUp = ((detail.LastAction == InpType::KEYDOWN) || (detail.LastAction == InpType::KEYREPEAT)) && (stroke.Flags & static_cast<WORD>(InpType::KEYUP));
			if (DoDown)
			{
				ControllerButtonToActionMap overtaken;
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
		/// <summary>Does the key send call, updates LastAction and updates LastSentTime</summary>
		void SendTheKey(ControllerButtonToActionMap& mp, const bool keyDown, ControllerButtonToActionMap::ActionType action) noexcept
		{
			mp.LastAction = action;
			m_key_send.SendScanCode(mp.MappedToVK, keyDown);
			mp.LastSentTime.Reset(m_ksp.settings.MICROSECONDS_DELAY_KEYREPEAT); // update last sent time
		}
		/// <summary>Check to see if a different axis of the same thumbstick has been pressed already</summary>
		/// <param name="detail">Newest element being set to keydown state</param>
		///	<param name="outOvertaken">out key set to the one being overtaken</param>
		/// <returns>true if is overtaking a thumbstick direction already depressed</returns>
		[[nodiscard]]
		auto IsOvertaking(const ControllerButtonToActionMap &detail, ControllerButtonToActionMap &outOvertaken) -> bool
		{
			//Is detail a thumbstick direction map, and if so, which thumbstick.
			const auto leftAxisIterator = std::ranges::find(m_ksp.settings.THUMBSTICK_L_VK_LIST, detail.SendingElementVK);
			const auto rightAxisIterator = std::ranges::find(m_ksp.settings.THUMBSTICK_R_VK_LIST, detail.SendingElementVK);
			const bool leftStick = leftAxisIterator != m_ksp.settings.THUMBSTICK_L_VK_LIST.end();
			const bool rightStick = rightAxisIterator != m_ksp.settings.THUMBSTICK_R_VK_LIST.end();
			//find a key-down'd or repeat'd direction of the same thumbstick
			if (leftStick || rightStick)
			{
				//build list of key-down state maps that match the thumbstick of the current "detail" map.
				const auto stickSettingList = leftStick ? m_ksp.settings.THUMBSTICK_L_VK_LIST : m_ksp.settings.THUMBSTICK_R_VK_LIST;
				auto TestFunc = [&stickSettingList, &detail](const ControllerButtonToActionMap& elem)
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
		/// <summary> Specific type of key send to send the input to handle the key-up that occurs
		///	when a thumbstick is rotated to the right angle to denote a new key. </summary>
		/// <param name="detail">Which key is being overtaken.</param>
		void DoOvertaking(ControllerButtonToActionMap &detail) noexcept
		{
			SendTheKey(detail, false, InpType::KEYUP);
		}
		/// <summary> Checks a <c>ControllerButtonToActionMap</c>'s <c>MappedToVK</c> and <c>SendingElementVK</c> members for out of bounds values. </summary>
		///	<returns> Error message on error, empty string otherwise. </returns>
		[[nodiscard]]
		auto CheckForVKError(const ControllerButtonToActionMap& detail) const -> std::string
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

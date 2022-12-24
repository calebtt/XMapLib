#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include <iostream>
#include <chrono>
#include <optional>

namespace sds
{
	/// <summary>
	///	Holds the functions used for mapping button -> keystroke.
	///	App-specific logic in the context of a generic controller button -> action library.
	///	TODO this may be moved to it's own project (app-specific stuff project).
	/// </summary>
	struct KeyboardTranslator
	{
		using InpType = ControllerButtonStateData::ActionType;
	public:
		KeyboardSettingsPack m_ksp;
		ControllerButtonToActionMap* m_currentMapping{};
	public:
		// Section for interface functions, factory funcs essentially.
		auto GetDownHandler(
			ControllerButtonStateData& lastState,
			const ControllerButtonData& cbd,
			const KeyStateWrapper& stroke,
			const std::vector<int>& vkList
		)
		{
			//TODO answer the simple question, exactly what data does this need to operate?

			//meow!
			return [&](
				ControllerButtonStateData &detail,
				ControllerButtonToActionMap& cbta, 
				const KeyStateWrapper& stroke) { Normal(detail, cbta, stroke); };
		}
		auto GetRepeatHandler()
		{
			
		}
		auto GetUpHandler()
		{
			
		}
	public:
		explicit KeyboardTranslator(ControllerButtonToActionMap& mapToOperateOn, const KeyboardSettingsPack& ksp = {})
			: m_ksp(ksp), m_currentMapping(&mapToOperateOn)
		{
		}
		/// <summary> If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--
		///	otherwise reset it immediately. </summary>
		static
		void KeyUpdateLoop(const std::vector<ControllerButtonToActionMap*> &mapBuffer)
		{
			//If enough time has passed, reset the key for use again, provided it uses the key-repeat behavior--
			//otherwise reset it immediately.
			for (const auto& elem : mapBuffer)
			{
				auto &cbData = elem->ControllerButtonState;
				const auto e = elem->KeymapData;
				const bool DoUpdate = (cbData.LastAction == InpType::KEYUP && cbData.LastSentTime.IsElapsed()) && e.UsesRepeat;
				const bool DoImmediate = cbData.LastAction == InpType::KEYUP && !e.UsesRepeat;
				if (DoUpdate || DoImmediate)
					cbData.LastAction = InpType::NONE;
			}
		}
		/// <summary> Checks each <c>ControllerButtonToActionMap</c> in <c>m_map_token_info</c>'s <c>LastSentTime</c> timer for being
		///	elapsed, and if so, sends the repeat keypress (if key repeat behavior is enabled for the map). </summary>
		void KeyRepeatLoop(const std::vector<ControllerButtonToActionMap*>& mapBuffer) const noexcept
		{
			using AT = InpType;
			for (const auto& w : mapBuffer)
			{
				const bool usesRepeat = w->KeymapData.UsesRepeat;
				const auto lastAction = w->ControllerButtonState.LastAction;
				if (usesRepeat && (lastAction == AT::KEYDOWN || lastAction == AT::KEYREPEAT))
				{
					if (w->ControllerButtonState.LastSentTime.IsElapsed())
						SendTheKey(*w, true, AT::KEYREPEAT);
				}
			}
		}
		/// <summary>Normal keypress simulation logic.</summary>
		void Normal(const KeyStateWrapper& stroke) const noexcept
		{
			constexpr auto noneType = InpType::NONE;
			constexpr auto downType = InpType::KEYDOWN;
			constexpr auto repeatType = InpType::KEYREPEAT;
			//const auto upType = InpType::KEYUP;

			auto lastAction = m_currentMapping->ControllerButtonState.LastAction;
			//const auto keymaps = detail.thisBuffer;
			//const auto controllerVk = detail.ControllerButton.VK;

			const bool AskDown = stroke.KeyDown;
			const bool DoDown = 
				(lastAction == repeatType && AskDown) || 
				(lastAction == downType && AskDown);
			const bool DoUp = (lastAction == downType
				|| lastAction == repeatType)
				&& !AskDown;

			if (DoDown)
			{
				const auto overtakenElem = IsOvertaking(bsd, detail, keymaps);
				if (overtakenElem.has_value())
				{
					DoOvertaking(*overtakenElem.value());
					SendTheKey(detail, bsd, true, InpType::KEYDOWN);
				}
				else
					SendTheKey(detail, bsd, true, InpType::KEYDOWN);
			}
			else if (DoUp)
			{
				SendTheKey(detail, bsd, false, InpType::KEYUP);
			}
		}
		/// <summary>Does the key send call, updates LastAction and updates LastSentTime</summary>
		void SendTheKey(
			ControllerButtonStateData& mp, 
			const ControllerButtonData& bsd,
			const std::vector<ControllerButtonToActionMap*>& keymaps,
			const bool keyDown, 
			ControllerButtonStateData::ActionType action) noexcept
		{
			for(auto &elem: keymaps)
			{
				if (elem->ControllerButtonState.LastAction == action)
				{
					elem.second();
					mp.ControllerButtonState.LastAction = action;
					mp.ControllerButtonState.LastSentTime.Reset(m_ksp.settings.MICROSECONDS_DELAY_KEYREPEAT);
				}
			}
		}
		/// <summary>Check to see if a different axis of the same thumbstick has been pressed already</summary>
		/// <param name="detail">Newest element being set to keydown state</param>
		/// <returns> optional, a <c>ControllerButtonToActionMap</c> pointer to the overtaken mapping, if is overtaking a thumbstick direction already depressed </returns>
		[[nodiscard]]
		auto IsOvertaking(const ControllerButtonToActionMap& detail) noexcept
		-> std::optional<ControllerButtonToActionMap*>
		{
			using std::ranges::all_of, std::ranges::find, std::ranges::find_if, std::ranges::begin, std::ranges::end;
			// Get copy of range to pointers to all mappings in existence.
			const auto mapBuffer = detail.GetThisBuffer();
			// Get copy of pointers to all mappings in the same exclusivity grouping as this mapping.
			const auto groupedBuffer = std::ranges::views::transform(mapBuffer, [exGroup = m_currentMapping->ExclusivityGrouping](const auto& elem)
				{
					return elem->ExclusivityGrouping == exGroup;
				});

			// If exclusivity grouping has some other members...
			if(!groupedBuffer.empty())
			{
				auto IsGroupedBtnPressedFn = [&](const ControllerButtonToActionMap* groupedButtonElem)
				{
					const auto elemState = groupedButtonElem->ControllerButtonState.LastAction;
					const auto elemButtonVK = groupedButtonElem->ControllerButton.VK;
					constexpr auto DownType = InpType::KEYDOWN;
					constexpr auto RepeatType = InpType::KEYREPEAT;
					const bool isOtherButtonPressed = elemState == DownType || elemState == RepeatType;
					// this is the case where the key we're testing for is the same key we're investigating
					const bool isSameKey = elemButtonVK == detail.ControllerButton.VK;
					if(!isSameKey)
					{
						return isOtherButtonPressed;
					}
					return false;
				};

				const auto mpit = find_if(mapBuffer, IsGroupedBtnPressedFn);
				if (mpit == end(mapBuffer))
					return {};
				return *mpit;
			}
			return {};
		}
		/// <summary> Sends the key-up for the overtaken key. Specific type of key send to send the input to handle the key-up that occurs
		///	when a thumbstick is rotated to the right angle to denote a new key. </summary>
		/// <param name="detail">Which key is being overtaken.</param>
		void DoOvertaking(ControllerButtonToActionMap& detail) const noexcept
		{
			SendTheKey(detail, false, InpType::KEYUP);
		}
		/// <summary> Checks a <c>ControllerButtonToActionMap</c>'s <c>MappedToVK</c> and <c>SendingElementVK</c> members for out of bounds values. </summary>
		///	<returns> Error message on error, empty optional otherwise. </returns>
		[[nodiscard]]
		auto CheckForVKError(const ControllerButtonToActionMap& detail) const -> std::optional<std::string>
		{
			const int vk = std::any_cast<int>(detail.KeyboardButton.VK);
			const auto& cbutton = detail.ControllerButton.VK;
			if ((vk <= 0) || (cbutton <= 0))
			{
				std::stringstream error;
				error << detail;
				return std::string("Contents:\n") + error.str() + "\nKeyboard VK and/or controller button VK out of range!";
			}
			return {};
		}
	};
}

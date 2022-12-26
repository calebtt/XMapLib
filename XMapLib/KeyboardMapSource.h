#pragma once
#include "stdafx.h"
#include <vector>

#include "ControllerButtonToActionMap.h"
#include "KeyboardTranslator.h"
#include "VirtualMap.h"

namespace sds
{
	/// <summary>
	/// Manages operating on a collection of controller button to <b>keyboard key</b> maps.
	///	It will be sure to add the callbacks that update the state machine (<c>KeyboardTranslator</c>) to keep track of the events.
	/// App specific logic used for the program here, specific to keyboard key mappings.
	/// </summary>
	struct KeyboardMapSource
	{
		using InpType = sds::ControllerButtonStateData::ActionType;
		using TranslatorFunctions_t = KeyboardTranslator;
	private:
		// non-owning pointer to a KeyboardTranslator object.
		TranslatorFunctions_t* m_pTranslator{};
		// List of controller btn to kbd key maps (this class is used explicitly to create
		// the keyboard key maps, not just controller btn to action).
		std::vector<ControllerButtonToActionMap> m_keyMaps;
		// keyboard settings pack
		KeyboardSettingsPack m_ksp;
		Utilities::SendKeyInput m_key_send;
	public:
		explicit KeyboardMapSource(TranslatorFunctions_t &kt) : m_pTranslator(&kt)
		{
			// Might also extract the logic in a form that is easier to use. Updating the state machine
			// would entail adding to the end of the task list for each functionality in ControllerButtonToActionMap.
			// TODO this class exists to properly construct the mapping objects explicitly for the case of controller btn to keyboard btn mappings, using the state machine.
		}
		~KeyboardMapSource()
		{
			//Cleanup in-progress key-presses.
			CleanupInProgressEvents();
		}
	public:
		auto BuildMapping(
			const int controllerVK,
			const int keyboardVK,
			ControllerToKeyMapData ctkmd = {})
		{
			// Add the various data packs
			ControllerButtonToActionMap cbtam;
			cbtam.ControllerButton.VK = controllerVK;
			cbtam.KeyboardButton.VK = keyboardVK;
			cbtam.KeymapData = std::move(ctkmd);
			cbtam.MappedActionsArray[InpType::KEYDOWN].PushInfiniteTaskBack([]() {});

			KeyboardTranslator kt{ std::move(cbtam), m_ksp };

			m_keyMaps.emplace_back(cbtam);

			//cbtam.KeymapData.ExclusivityGrouping = exclusivityGrouping;
			//cbtam.KeymapData.ExclusivityNoOvertakingGrouping = exclusivityGroupingNoUpdate;
			//cbtam.KeymapData.UsesRepeat = usesRepeat;
			//cbtam.KeymapData.DelayAfterRepeatActivation = delayAfterRepeat;
			//
			
			// TODO continue here.

			//cbtam.ControllerButton = cbd;
			//cbtam.KeyboardButton = kbd;
			//cbtam.KeymapData = ctkmd;
			//// Add the app-specific logic for keyboard mappings.
			//cbtam.MappedActionsArray[InpType::KEYDOWN].PushInfiniteTaskBack(
			//	[trns = m_pTranslator](ControllerButtonToActionMap &cbta, const ControllerStateWrapper &stroke ) { trns->DoDown(cbta, stroke); }
			//);
			//cbtam.MappedActionsArray[InpType::KEYREPEAT].PushInfiniteTaskBack(
			//	[trns = m_pTranslator](ControllerButtonToActionMap& cbta, const ControllerStateWrapper& stroke) { trns->DoDown(cbta, stroke); }
			//);
		}
		/// <summary><c>AddMap(ControllerButtonToActionMap)</c> Adds a key map.</summary>
		void AddMap(const ControllerButtonToActionMap button)
		{
			m_keyMaps.emplace_back(button);
		}
		[[nodiscard]]
		auto GetMaps() const noexcept -> std::vector<ControllerButtonToActionMap>
		{
			return m_keyMaps;
		}
		void ClearMaps() noexcept
		{
			CleanupInProgressEvents();
			m_keyMaps.clear();
		}
	private:
		/// <summary>Call this function to send key-ups for any in-progress key presses.</summary>
		void CleanupInProgressEvents() const
		{
			using std::ranges::find_if, std::ranges::end;
			for (const ControllerButtonToActionMap& m : m_keyMaps)
			{
				const auto& la = m.ControllerButtonState.LastAction;
				if (la == InpType::KEYDOWN || la == InpType::KEYREPEAT)
				{
					// Find the keyup callback range.
					const auto act = find_if(m.MappedActionsArray, [&](const auto& e) { return e.first == InpType::KEYUP; });
					// assert that it actually found the state
					assert(act != end(m.MappedActionsArray));
					// call the callback range functions for keyup
					act->second();
				}
			}
		}
	};
}
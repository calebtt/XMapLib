#pragma once
#include "stdafx.h"
#include <vector>

#include "ControllerButtonToActionMap.h"
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
	private:
		// non-owning pointer to a KeyboardTranslator object.
		KeyboardTranslator* m_pTranslator{};
		// List of controller btn to kbd key maps (this class is used explicitly to create
		// the keyboard key maps, not just controller btn to action).
		std::vector<ControllerButtonToActionMap> m_keyMaps;
		// keyboard settings pack
		KeyboardSettingsPack m_ksp;
		Utilities::SendKeyInput m_key_send;
	public:
		explicit KeyboardMapSource(KeyboardTranslator &kt) : m_pTranslator(&kt)
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
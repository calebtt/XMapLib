#pragma once
#include "stdafx.h"
#include <vector>

#include "ControllerButtonToActionMap.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardTranslator.h"
#include "VirtualMap.h"

namespace sds
{
	//TODO might overtake some of the responsibility of the stuff in ControllerButtonToActionMap
	// Use that one design pattern where an instance is also a collection of the instance type.
	// because I want it to be mutated outside of the class as a collection, but aggregate operations
	// for the most part won't make sense. Still have that ability if required.

	//TODO updating the state machine as these tasks are performed needs a clever solution.

	/// <summary> Manages creation of and operating on a collection of controller button to keyboard key maps.
	///	It will be sure to add the callbacks that update the state machine (KeyboardTranslator) to keep track of the events.
	/// App specific logic. </summary>
	struct KeyboardMapSource
	{
		using InpType = sds::ControllerButtonToActionMap::ActionType;
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
		auto AddMap(const char mappedFromChar, const int controllerVk, const bool useRepeat)
		{
			// Get Virtual Keycode from char.
			const auto vkOfPrintable = static_cast<int>(Utilities::VirtualMap::GetVKFromChar(mappedFromChar));
			ControllerButtonToActionMap cbtam{ controllerVk, vkOfPrintable, true };
			cbtam.ActivationTasks.PushInfiniteTaskBack([&]() { m_pTranslator->OnKeyDown(cbtam); });
			cbtam.DeactivationTasks.PushInfiniteTaskBack([&]() { m_pTranslator->OnKeyUp(cbtam); });
			if(useRepeat)
				cbtam.RepeatTasks.PushInfiniteTaskBack([&]() { m_pTranslator->OnKeyRepeat(cbtam); });
			// Add the map to the internal list.
			m_keyMaps.emplace_back(cbtam);
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
			for (auto& m : m_keyMaps)
			{
				if (m.LastAction == InpType::KEYDOWN || m.LastAction == InpType::KEYREPEAT)
				{
					m.CleanupTasks();
				}
			}
		}
	};
}
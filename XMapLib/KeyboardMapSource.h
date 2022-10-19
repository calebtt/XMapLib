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
	public:
		// PUBLIC data member, accessible directly.
		std::vector<ControllerButtonToActionMap> KeyMaps;
		// PUBLIC data member, keyboard settings.
		KeyboardSettingsPack m_ksp;
		KeyboardTranslator* m_pTranslator;
	public:
		KeyboardMapSource(KeyboardTranslator &kt) : m_pTranslator(&kt)
		{
			//TODO store a non-owning pointer to this. It provides the state machine logic.
			// Might also extract the logic in a form that is easier to use. Updating the state machine
			// would entail adding to the end of the task list for each functionality in ControllerButtonToActionMap.
			// TODO this class exists to properly construct the mapping objects explicitly for the case of controller btn to keyboard btn mappings, using the state machine.
		}
	public:
		/// <summary><c>AddMap(ControllerButtonToActionMap)</c> Adds a key map.</summary>
		void AddMap(const ControllerButtonToActionMap button)
		{
			KeyMaps.emplace_back(button);
		}
		auto AddMap(const char mappedFromChar, const int controllerVk)
		{
			// Get Virtual Keycode from char.
			const auto vkOfPrintable = static_cast<int>(Utilities::VirtualMap::GetVKFromChar(mappedFromChar));
			ControllerButtonToActionMap cbtam{ controllerVk, vkOfPrintable, true };
			auto sendingFn = [=, this]()
			{
				SendTheKey(cbtam, true, ControllerButtonToActionMap::ActionType::KEYDOWN);
			};
			//cbtam.ActivationTasks.PushInfiniteTaskBack();

		}
		[[nodiscard]]
		auto GetMaps() const noexcept -> std::vector<ControllerButtonToActionMap>
		{
			return KeyMaps;
		}
		void ClearMaps() noexcept
		{
			KeyMaps.clear();
		}
	private:
		Utilities::SendKeyInput m_key_send;
	private:
		/// <summary>Does the key send call, updates LastAction and updates LastSentTime</summary>
		void SendTheKey(ControllerButtonToActionMap &mp, const bool keyDown, ControllerButtonToActionMap::ActionType action) noexcept
		{
			mp.LastAction = action;
			m_key_send.SendScanCode(mp.MappedToVK, keyDown);
			mp.LastSentTime.Reset(m_ksp.settings.MICROSECONDS_DELAY_KEYREPEAT); // update last sent time
		}
	};
}
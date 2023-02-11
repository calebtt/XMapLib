#pragma once
#include "stdafx.h"
#include <vector>

#include "ControllerButtonToActionMap.h"
#include "KeyboardTranslator.h"
#include "VirtualMap.h"

namespace sds
{
	/**
	 * \brief Manages operating on a collection of controller button to <b>keyboard key</b> maps. It will be sure to add the callbacks that
	 * update the state machine (<c>KeyboardTranslator</c>) to keep track of the events. App specific logic used for the program here, specific to keyboard key mappings.
	 */
	struct KeyboardMapSource
	{
		using InpType = sds::ControllerButtonStateData::ActionType;
		using CBTAM_t = ControllerButtonToActionMap<>;
		using MapBuffer_t = std::vector<CBTAM_t>;
	private:
		// List of controller btn to kbd key maps (this class is used explicitly to create
		// the keyboard key maps, not just controller btn to action).
		MapBuffer_t m_keyMaps;
		// Input simulation provider
		Utilities::SendKeyInput m_keySend;
	public:
		KeyboardMapSource() = default;
		~KeyboardMapSource() = default;

		auto ProcessState(const ControllerStateWrapper& detail)
		{
			for (auto& elem : m_keyMaps)
				elem.ProcessState(detail);
		}
		/**
		 * \brief Constructs a mapping and adds it to the internal collection.
		 * \param controllerVK VK of controller button
		 * \param keyboardVK VK of keyboard key
		 * \param ctkmd optional, additional key mapping info
		 */
		auto AddMap(
			const int controllerVK,
			const int keyboardVK,
			ControllerToKeyMapData ctkmd = {})
		{
			// Add the various data packs
			CBTAM_t cbtam;
			cbtam.ControllerButton.VK = controllerVK;
			cbtam.KeyboardButton.VK = keyboardVK;
			cbtam.KeymapData = std::move(ctkmd);
			cbtam.MappedActionsArray[InpType::KEYDOWN].PushInfiniteTaskBack([&]()
				{
					m_keySend.SendScanCode(keyboardVK, true);
				});
			cbtam.MappedActionsArray[InpType::KEYUP].PushInfiniteTaskBack([&]()
				{
					m_keySend.SendScanCode(keyboardVK, false);
				});
			cbtam.MappedActionsArray[InpType::KEYREPEAT].PushInfiniteTaskBack([&]()
				{
					m_keySend.SendScanCode(keyboardVK, true);
				});
			m_keyMaps.emplace_back(cbtam);
		}

		/**
		 * \brief Adds a previously constructed mapping to the internal collection.
		 * \param button Previously constructed mapping.
		 */
		auto AddMap(const CBTAM_t& button)
		{
			m_keyMaps.emplace_back(button);
		}

		[[nodiscard]]
		auto GetMaps() const noexcept -> MapBuffer_t
		{
			return m_keyMaps;
		}

		void ClearMaps() noexcept
		{
			CleanupInProgressEvents();
			m_keyMaps.clear();
		}

		/**
		 * \brief Call this function to send key-ups for any in-progress key presses.
		 */
		void CleanupInProgressEvents() const
		{
			using std::ranges::find_if, std::ranges::end;
			for (const CBTAM_t& m : m_keyMaps)
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
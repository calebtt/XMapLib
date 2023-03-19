#pragma once
#include "ControllerButtonToActionMap.h"
#include "KeyboardTranslator.h"

namespace sds
{
	inline
	auto DoKeyUpForMatchingExclusivityGroupMappings(sds::CBActionMap mapping, auto& mappingsList)
	{
		// Handle exclusivity grouping members, they get sent key-up if they match this key-down's ex. group
		if (mapping.ExclusivityGrouping.has_value())
		{
			// Get a view to the elements matching the same exclusivity grouping.
			const auto rv = GetMatchingExclusivityGroupMappings(mapping, mappingsList);
			// Send key-up for the matching ex. group members.
			for (auto& et : rv)
			{
				if (et->second.IsDown() || et->second.IsRepeating())
				{
					// Send the up
					if (et->first.OnUp)
						et->first.OnUp.value()();
					// Update last sent time
					et->second.LastSentTime.Reset();
					// Update the last state
					et->second.SetUp();
				}
			}
		}
	}
	/**
	 * \brief DOES NOT CHECK THE STATE or the TIMER before sending, merely does the send & update logic.
	 *  NOTE that here, sending the key-ups for all the overtaken mappings in the same exclusivity grouping counts as "UPDATE"
	 */
	inline
	auto SendAndUpdateKeyDown(std::pair<sds::CBActionMap, sds::MappingStateManager>& mapping, auto& mappingsList)
	{
		DoKeyUpForMatchingExclusivityGroupMappings(mapping.first, mappingsList);

		// test for handler
		if (mapping.first.OnDown)
			mapping.first.OnDown.value()();
		// Update the last state
		mapping.second.SetDown();
		// Update last sent time
		mapping.second.LastSentTime.Reset();
	}
	/**
	 * \brief DOES NOT CHECK THE STATE or the TIMER before sending, merely does the send & update logic.
	 */
	inline
	auto SendAndUpdateKeyRepeat(sds::CBActionMap& mapping)
	{
		// note, it should not be possible for a repeat to trigger an exclusivity grouping overtaking action afaik
		// test for handler
		if (mapping.OnRepeat)
			mapping.OnRepeat.value()();

		// Update the last state
		mapping.LastAction.SetRepeat();

		// Update last sent time
		mapping.LastAction.LastSentTime.Reset();
	}
	/**
	 * \brief DOES NOT CHECK THE STATE or the TIMER before sending, merely does the send & update logic.
	 */
	inline
	auto SendAndUpdateKeyUp(sds::CBActionMap& mapping)
	{
		// test for handler
		if (mapping.OnUp)
			mapping.OnUp.value()();

		mapping.LastAction.SetUp();

		// Update last sent time
		mapping.LastAction.LastSentTime.Reset();
	}
}
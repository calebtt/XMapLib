#pragma once
#include "LibIncludes.h"
#include "CustomTypes.h"

#include <iostream>
#include <chrono>
#include <vector>
#include <ostream>
#include <ranges>

namespace sds
{
	/**
	 * \brief	TranslationResult holds info from a translated state change, typically the operation to perform (if any) and
	 *	a function to call to advance the state to the next state to continue to receive proper translation results.
	 */
	struct TranslationResult
	{
		//TODO add meta-data about the mapping, like which VK it's mapped to, the user could build their own mechanism to acquire
		//this info, but it's much easier to just add it here in the event it could be useful.

		// Action to perform
		ActionState DoState;
		// Operation being requested to be performed, callable
		detail::Fn_t OperationToPerform;
		// Function to advance the button mapping to the next state (after operation has been performed)
		detail::Fn_t AdvanceStateFn;
		// Call operator, calls op fn then advances the state
		void operator()() const
		{
			OperationToPerform();
			AdvanceStateFn();
		}
		// Debugging purposes
		friend auto operator<<(std::ostream& os, const TranslationResult& obj) -> std::ostream&
		{
			return os
				<< "DoState: " << std::boolalpha << (int)obj.DoState;
		}
	};

	/**
	 * \brief	TranslationPack is a pack of ranges containing individual TranslationResult structs for processing
	 *	state changes.
	 */
	struct TranslationPack
	{
		void operator()() const
		{
			// Note that there will be a function called if there is a state change,
			// it just may not have any custom behavior attached to it.
			for (const auto& elem : UpdateRequests)
				elem();
			for (const auto& elem : OvertakenRequests)
				elem();
			for (const auto& elem : RepeatRequests)
				elem();
			for (const auto& elem : NextStateRequests)
				elem();
		}
		// TODO might wrap the vectors in a struct with a call operator to have individual call operators for range of TranslationResult.
		std::vector<TranslationResult> UpdateRequests;
		std::vector<TranslationResult> RepeatRequests;
		std::vector<TranslationResult> OvertakenRequests;
		std::vector<TranslationResult> NextStateRequests;
	};

}

#pragma once
#include "LibIncludes.h"
#include "ButtonStateMgr.h"

#include <iostream>
#include <chrono>
#include <vector>
#include <ostream>
#include <ranges>

namespace sds
{
	struct TranslationResult
	{
		//TODO add meta-data about the mapping, like which VK it's mapped to, the user could build their own mechanism to acquire
		//this info, but it's much easier to just add it here in the event it could be useful.

		// Action to perform
		ButtonStateMgr DoState;
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
				<< "DoDown: " << std::boolalpha << obj.DoState.IsDown()
				<< " DoRepeat: " << std::boolalpha << obj.DoState.IsRepeating()
				<< " DoUp: " << std::boolalpha << obj.DoState.IsUp()
				<< " DoReset: " << std::boolalpha << obj.DoState.IsInitialState();
		}
	};

	struct TranslationPack
	{
		void operator()() const
		{
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
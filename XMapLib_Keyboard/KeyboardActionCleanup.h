#pragma once
#include "LibIncludes.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPoller.h"
#include <iostream>
#include <chrono>
#include <optional>
#include <vector>
#include <iterator>
#include <ostream>
#include <tuple>
#include <ranges>
#include <type_traits>

namespace sds
{
	/**
	 * \brief Class that provides cleanup actions for in-progress events.
	 */
	class KeyboardActionCleanup
	{
	private:
		std::vector<CBActionMap> m_mappings;
		std::map<int, std::vector<CBActionMap*>> m_exGroupMap;
	public:

	};
}
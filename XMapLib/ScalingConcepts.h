#pragma once
#include "stdafx.h"
#include <type_traits>
#include <ranges>

namespace sds
{
	// Concept for a type that is intended to be used to read or otherwise return
	// a list of scaling values, inside a range compatible container, as an array of floating point type
	// Used by ThumbstickToDelay
	template<typename T>
	concept ReadScalesType = requires(T & t)
	{
		{ std::is_default_constructible_v<typename T::FloatType> };
		{ std::is_default_constructible_v<typename T::RangeType> };
		{ t.GetScalingValues() } -> std::convertible_to<typename T::RangeType>;
		{ T("Some File Name") };
		{ std::ranges::range<typename T::RangeType> };
	};
}
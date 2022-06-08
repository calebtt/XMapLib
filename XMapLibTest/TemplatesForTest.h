#pragma once
#include <cmath>
#include "pch.h"
#include "CppUnitTest.h"

//using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Utility functions used in testing.
namespace XMapLibTest::TemplatesForTest
{
	//concept for being an integral or floating point type
	template<class T>
	concept is_number_v = std::is_integral_v<T> || std::is_floating_point_v<T>;
	//concept for having the possibility of being a negative
	template<class T>
	concept has_neg_possible = std::floating_point<T> || (!std::unsigned_integral<T>);

	constexpr bool IsWithin(const auto result, const auto testVal, const auto within)
	{
		return ((result > (testVal - within)) && (result < (testVal + within)));
	}
	// Replacement for the much longer "static_cast<>"
	template<typename T>
	[[nodiscard]] constexpr T ToA(const is_number_v auto something) noexcept
	{
		return static_cast<T>(something);
	}

	constexpr bool IsNormalF(const auto val)
	{
		return std::isnormal(static_cast<float>(val));
	};
}
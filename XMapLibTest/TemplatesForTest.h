#pragma once
#include <cmath>
#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace XMapLibTest::TemplatesForTest
{
	bool IsWithin(const auto result, const auto testVal, const auto within)
	{
		return ((result > (testVal - within)) && (result < (testVal + within)));
	}
	constexpr double ToDub(const auto something)
	{
		return static_cast<double>(something);
	}
	constexpr float ToFloat(const auto something)
	{
		return static_cast<float>(something);
	}
	constexpr int ToInt(const auto something)
	{
		return static_cast<int>(something);
	}
	constexpr long long ToLL(const auto something)
	{
		return static_cast<long long>(something);
	}
	constexpr bool IsNormalF(const auto val)
	{
		return std::isnormal(static_cast<float>(val));
	};
}
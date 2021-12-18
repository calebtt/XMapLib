#pragma once
#include <cmath>
namespace sds::Utilities
{
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
	constexpr bool IsNormalF(const auto val)
	{
		return std::isnormal(static_cast<float>(val));
	};
}

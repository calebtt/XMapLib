#pragma once
#include <cmath>
namespace sds::Utilities
{
	template<class T>
	concept is_number_v = std::is_integral_v<T> || std::is_floating_point_v<T>;
	constexpr double ToDub(const is_number_v auto something) noexcept
	{
		return static_cast<double>(something);
	}
	constexpr float ToFloat(const is_number_v auto something) noexcept
	{
		return static_cast<float>(something);
	}
	constexpr int ToInt(const is_number_v auto something) noexcept
	{
		return static_cast<int>(something);
	}
	constexpr bool IsNormalF(const is_number_v auto val) noexcept
	{
		return std::isnormal(static_cast<float>(val));
	}
}

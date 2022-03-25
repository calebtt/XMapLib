#pragma once
#include <cmath>
namespace sds::Utilities
{
	template<class T>
	concept is_number_v = std::is_integral_v<T> || std::is_floating_point_v<T>;

	template<typename T>
	[[nodiscard]] constexpr T ToA(const is_number_v auto something) noexcept
	{
		return static_cast<T>(something);
	}
	[[nodiscard]] constexpr bool IsNormalF(const is_number_v auto val) noexcept
	{
		return std::isnormal(static_cast<float>(val));
	}
	/// <summary> Absolute value replacement function. This version is constexpr! </summary>
	/// <param name="val">Number value to perform absolute value on</param>
	/// <returns>Returns the absolute value of val</returns>
	[[nodiscard]] constexpr auto ConstAbs(const is_number_v auto val) noexcept
	{
		constexpr decltype(val) zeroValue{};
		if (val < zeroValue)
		{
			return (-val);
		}
		return val;
	}
}

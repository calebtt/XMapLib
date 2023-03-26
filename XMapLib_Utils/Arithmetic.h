#pragma once
#include <cmath>
#include <type_traits>
#include <utility>

namespace sds::Utilities
{
	//concept for being an integral or floating point type
	template<class T>
	concept is_number_v = std::is_integral_v<T> || std::is_floating_point_v<T>;
	//concept for having the possibility of being a negative
	template<class T>
	concept has_neg_possible = std::floating_point<T> || (!std::unsigned_integral<T>);
	//concept for having NO possibility of being a negative
	template<class T>
	concept has_no_neg_possible = (!std::floating_point<T>) && (std::unsigned_integral<T>);

	/// <summary> Casts <c>something</c> to a type T but with a shorter syntax than
	///	static_cast. </summary>
	/// <typeparam name="T">Type to cast TO</typeparam>
	/// <param name="something">Variable to be casted.</param>
	/// <returns>variable as type T</returns>
	template<typename T>
	[[nodiscard]] constexpr T ToA(const is_number_v auto something) noexcept
	{
		return static_cast<T>(something);
	}
	/// <summary> Returns true if val is "normal" i.e., neither zero, subnormal, infinite, nor NaN. Explicitly calls the <c>float</c> version of the function. </summary>
	[[nodiscard]] constexpr bool IsNormalF(const is_number_v auto val) noexcept
	{
		return std::isnormal(static_cast<float>(val));
	}
	/// <summary> Absolute value replacement function. This version is constexpr! </summary>
	/// <param name="val">Number value to perform absolute value on</param>
	/// <returns>Returns the absolute value of val</returns>
	///	<remarks> Unrepresentable negative values will be returned as the type's maximum value! </remarks>
	[[nodiscard]] constexpr auto ConstAbs(const auto val) noexcept
	{
		if constexpr (std::unsigned_integral<decltype(val)>)
		{
			return val;
		}
		else
		{
			// This is a runtime check of a boolean provided at compile time.
			if (!std::is_constant_evaluated())
				return std::abs(val);
			//For integral types (not floating point) we can test perfect equality to the min value
			//and assign the absolute value of it as the num limit max to avoid pesky overflows.
			if constexpr (std::integral<decltype(val)>)
			{
				//Check to see the value fits into the type as a positive.
				constexpr auto MinVal = std::numeric_limits<decltype(val)>::min();
				if (val == MinVal)
					return std::numeric_limits<decltype(val)>::max();
			}
			constexpr decltype(val) zeroValue{};
			return (val >= zeroValue) ? val : -val;
		}
	}
	/// <summary> Utility function for computing a non-negative inverse of a floating type percentage. </summary>
	[[nodiscard]] constexpr auto GetInverseOfPercentage(const auto scale) noexcept -> double
	{
		const auto invP = (1.0 - scale);
		if (invP < 0.0)
			return 0.0;
		return invP;
	}

	/// <summary> Utility function for computing a non-negative inverse of a floating type percentage plus 1.0 </summary>
	/// <remarks> NOTE: that compiling this is probably irreparably broken in VS2019 (or platform toolset 142 instead of 143),
	///try changing them to normal templates. </remarks>
	[[nodiscard]] constexpr auto GetInverseOfPercentagePlusOne(const auto scale) noexcept -> decltype(GetInverseOfPercentage(scale))
	{
		return GetInverseOfPercentage(scale) + 1.0;
	}

	/// <summary> Utility function for computing a non-negative float percentage. </summary>
	[[nodiscard]] constexpr auto GetPercentage(const double numerator, const double denominator) noexcept
	{
		const auto P = numerator / denominator;
		if (P < 0.0)
			return 0.0;
		return P;
	}
}
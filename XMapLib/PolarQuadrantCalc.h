#pragma once
#include "stdafx.h"

namespace sds
{
	class PolarQuadrantCalc
	{
	private:
		//number of coordinate plane quadrants (obviously 4)
		static constexpr auto NUM_QUADRANTS = 4;
		//double
		using FloatingType = double;
		//used to trim the adjusted magnitude values from the thumbstick, max on my hardware close to this number.
		static constexpr FloatingType MAGNITUDE_SENTINEL{ MouseSettings::ThumbstickValueMax };
		static constexpr FloatingType MY_PI{ std::numbers::pi };
		static constexpr FloatingType MY_TAU{ std::numbers::pi / 2 };

		//array of boundary values, used to determine which polar quadrant a polar angle resides in
		static constexpr std::array<const std::pair<FloatingType, FloatingType>, NUM_QUADRANTS> m_quadArray{
			std::make_pair(0.0, MY_TAU),
			std::make_pair(MY_TAU, MY_PI),
			std::make_pair(-MY_PI, -MY_TAU),
			std::make_pair(-MY_TAU, 0.0)
		};
	public:
		// [Pair[FloatingType,FloatingType], int] wherein the quadrant_range pair is the quadrant range, and the outer int quadrant_number is the quadrant number.
		struct QuadrantInfoPack
		{
			std::pair<FloatingType, FloatingType> quadrant_range{};
			int quadrant_number{};
		};
		//Pair[FloatingType, FloatingType] wherein the first member is the polar radius, and the second is the polar theta angle.
		struct PolarInfoPack
		{
			FloatingType polar_radius{};
			FloatingType polar_theta_angle{};
		};
		//Pair[FloatingType, FloatingType] wherein the first member is the adjusted X magnitude value, and the second is the adjusted Y magnitude
		struct AdjustedMagnitudePack
		{
			FloatingType x_adjusted_mag{};
			FloatingType y_adjusted_mag{};
		};
		struct PolarCompleteInfoPack
		{
			PolarInfoPack polar_info{};
			QuadrantInfoPack quadrant_info{};
			AdjustedMagnitudePack adjusted_magnitudes{};
		};
		[[nodiscard]] static PolarCompleteInfoPack ComputePolarCompleteInfo(const FloatingType xStickValue, const FloatingType yStickValue) noexcept
		{
			PolarCompleteInfoPack tempPack{};
			tempPack.polar_info = ComputePolarPair(xStickValue, yStickValue);
			tempPack.quadrant_info = GetQuadrantInfo(tempPack.polar_info.polar_theta_angle);
			tempPack.adjusted_magnitudes = ComputeAdjustedMagnitudes(tempPack.polar_info, tempPack.quadrant_info);
			return tempPack;
		}
		//compute adjusted magnitudes
		[[nodiscard]] static AdjustedMagnitudePack ComputeAdjustedMagnitudes(const PolarInfoPack polarInfo, const QuadrantInfoPack quadInfo) noexcept
		{
			const auto& [polarRadius, polarTheta] = polarInfo;
			const auto& [quadrantSentinelPair, quadrantNumber] = quadInfo;
			const auto& [quadrantBeginVal, quadrantSentinelVal] = quadrantSentinelPair;
			//compute proportion of the radius for each axis to be the axial magnitudes, apparently a per-quadrant calculation with my setup.
			const auto redPortion = (polarTheta - quadrantBeginVal) * polarRadius;
			const auto blackPortion = (quadrantSentinelVal - polarTheta) * polarRadius;
			const double xProportion = quadrantNumber % 2 ? blackPortion : redPortion;
			const double yProportion = quadrantNumber % 2 ? redPortion : blackPortion;
			return TrimMagnitudeToSentinel(xProportion, yProportion);
		}
		//compute polar coord pair
		[[nodiscard]] static PolarInfoPack ComputePolarPair(const FloatingType xStickValue, const FloatingType yStickValue) noexcept
		{
			constexpr FloatingType nonZeroValue = 0.01; // cannot compute with both values at 0, this is used instead
			const bool areBothZero = xStickValue == 0.0 && yStickValue == 0.0;
			const double xValue = areBothZero ? nonZeroValue : xStickValue;
			const double yValue = areBothZero ? nonZeroValue : yStickValue;
			const auto xSquared = xValue * xValue;
			const auto ySquared = yValue * yValue;
			const auto rad = std::sqrt(xSquared + ySquared);
			const auto angle = std::atan2(yValue, xValue);
			return { .polar_radius = rad, .polar_theta_angle = angle };
		}
		/// <summary> Retrieves begin and end range values for the quadrant the polar theta (angle) value resides in, and the quadrant number (NOT zero indexed!) </summary>
		/// <returns> Pair[Pair[double,double], int] wherein the inner pair is the quadrant range, and the outer int is the quadrant number. </returns>
		[[nodiscard]] static QuadrantInfoPack GetQuadrantInfo(const FloatingType polarTheta) noexcept
		{
			constexpr std::string_view BAD_QUAD = "Invalid value that does not map to a quadrant in GetQuadrantInfo(const FloatingType)";
			size_t index{};
			//Find polar theta value's place in the quadrant range array.
			const auto quadrantResult = std::ranges::find_if(m_quadArray, [&](const auto val)
				{
					++index;
					return (polarTheta >= std::get<0>(val) && polarTheta <= std::get<1>(val));
				});
			//This should not happen, but if it does, I want some kind of message about it.
			if (quadrantResult == m_quadArray.end())
			{
				Utilities::LogError(BAD_QUAD);
				return { {0.0,0.0}, -1 };
			}
			return { .quadrant_range = (*quadrantResult), .quadrant_number = static_cast<int>(index) };
		}
	private:
		//trim computed magnitude values to sentinel value
		[[nodiscard]] static constexpr AdjustedMagnitudePack TrimMagnitudeToSentinel(const FloatingType x, const FloatingType y) noexcept
		{
			auto tempX = x;
			auto tempY = y;
			tempX = std::clamp(tempX, -MAGNITUDE_SENTINEL, MAGNITUDE_SENTINEL);
			tempY = std::clamp(tempY, -MAGNITUDE_SENTINEL, MAGNITUDE_SENTINEL);
			return { tempX, tempY };
		}
	};
}
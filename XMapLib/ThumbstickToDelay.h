#pragma once
#include "PolarCalc.h"
#include "stdafx.h"
#include "Utilities.h"
#include "ReadRadiusScaleValues.h"

namespace sds
{
	/// <summary>Basic logic for mapping thumbstick values to work thread delay values.
	/// A single instance for a single thumbstick axis is to be used.
	/// This class must be re-instantiated to use new deadzone values.</summary>
	class ThumbstickToDelay
	{
		using MultFloat = decltype(MouseSettings::ALT_DEADZONE_MULT_DEFAULT);
		using SensInt = decltype(MouseSettings::SENSITIVITY_DEFAULT);
		using DzInt = decltype(MouseSettings::DEADZONE_DEFAULT);
		using PRadInt = decltype(MouseSettings::PolarRadiusValueMax);
		using ScaleFloat = ReadRadiusScaleValues::FloatType;
		using ScaleRange = ReadRadiusScaleValues::RangeType;
		//***
		//TODO use an unordered_map to hashmap angle to computed value,
		//And even pre-compute them eventually (all values).
		//***
	public:
	private:
		// Additional multiplier applied to polar radius scaling values for cartesian adjustments.
		static constexpr double AdditionalMultiplier{ 1.36 };
		// Axis sensitivity, 1-100
		const SensInt m_axis_sensitivity;
		// Denotes left stick, right stick, or neither.
		const StickMap m_which_stick;
		// cref to MouseSettings info struct, contains some program information occasionally used.
		const MouseSettings &m_mouse_settings;
		// Container holding polar radius scaling values, index is an integer representation of the
		// theta angle for 1st quadrant, 0 to 157
		const ScaleRange m_radius_scale_values;
		// Utility class for performing polar coordinate calculations.
		PolarCalc m_pc;

		///<summary> Used to make some assertions about the settings values this class depends upon. </summary>
		static void AssertSettings(const MouseSettings &ms) noexcept
		{
			//Assertions about the settings values used by this class.
			if constexpr (ms.MICROSECONDS_MIN >= ms.MICROSECONDS_MAX)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, MICROSECONDS_MIN >= MICROSECONDS_MAX");
			if constexpr (ms.SENSITIVITY_MIN >= ms.SENSITIVITY_MAX)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MIN >= SENSITIVITY_MAX");
			if constexpr (ms.SENSITIVITY_MIN <= 0)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MIN <= 0");
			if constexpr (ms.SENSITIVITY_MAX > 100)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MAX > 100");
		}
		/// <summary> Validates StickMap ctor args, neither stick is not a valid setting
		///	for this class, it will default to right stick processing when used. </summary>
		[[nodiscard]] static constexpr auto ValidateStickMap(const StickMap sm)  noexcept
		{
			//error checking StickMap stick setting
			if (sm == StickMap::NEITHER_STICK)
			{
				Utilities::LogError("Error in ThumbstickToDelay::ValidateStickMap(StickMap): NEITHER_STICK selected stick, choosing RIGHT_STICK.");
				return StickMap::RIGHT_STICK;
			}
			return sm;
		}
		/// <summary> Used to validate sensitivity arg value to within range SENSITIVITY_MIN to SENSITIVITY_MAX in MouseSettings. </summary>
		[[nodiscard]] static constexpr auto ValidateSensitivity(const SensInt si, const MouseSettings &ms)  noexcept
		{
			//range bind sensitivity
			return std::clamp(si, ms.SENSITIVITY_MIN, ms.SENSITIVITY_MAX);
		}
		/// <summary> Validation func for retrieving the polar angle scaling values from config file. </summary>
		[[nodiscard]] static auto ValidateScaleValues(const MouseSettings &ms) noexcept
		{
			//read radius scale values config file
			ReadRadiusScaleValues rrsv(ms);
			auto tempValues = rrsv.GetScalingValues();
			if (tempValues.empty())
				Utilities::LogError("Error in ThumbstickToDelay::ValidateScaleValues(MouseSettings), failed to read radius scaling values from config file!");
			return tempValues;
		}
	public:
		/// <summary> Constructor for single axis sensitivity and deadzone processing. </summary>
		/// <exception cref="string"> logs text string if MouseSettings values are unusable. </exception>
		/// <param name="sensitivity">int sensitivity value</param>
		/// <param name="whichStick">StickMap enum denoting which thumbstick</param>
		///	<param name="ms">Mouse Settings struct containing some default values</param>
		ThumbstickToDelay(
			const int sensitivity, 
			const StickMap whichStick,
			const MouseSettings &ms = {}) noexcept
		: m_axis_sensitivity(ValidateSensitivity(sensitivity, ms)),
		m_which_stick(ValidateStickMap(whichStick)),
		m_mouse_settings(ms),
		m_radius_scale_values(ValidateScaleValues(ms)),
		m_pc(ms.PolarRadiusValueMax, Utilities::LogError)
		{
			AssertSettings(ms);
		}
		ThumbstickToDelay() = delete;
		ThumbstickToDelay(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay(ThumbstickToDelay&& other) = delete;
		ThumbstickToDelay& operator=(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay& operator=(ThumbstickToDelay&& other) = delete;
		~ThumbstickToDelay() = default;
		///<summary> Utility function for computing a non-negative inverse of a float percentage plus 1.0 </summary>
		[[nodiscard]] constexpr auto GetInverseOfPercentagePlusOne(const auto scale) const noexcept
		{
			return GetInverseOfPercentage(scale) + 1.0;
		}
		///<summary> Utility function for computing a non-negative inverse of a float percentage plus 1.0 </summary>
		[[nodiscard]] constexpr auto GetInverseOfPercentage(const auto scale) const noexcept
		{
			const auto invP = (1.0 - scale);
			if (invP < 0.0)
				return 0.0;
			return invP;
		}
		///<summary> Utility function for computing a non-negative float percentage. </summary>
		[[nodiscard]] constexpr auto GetPercentage(const double numerator, const double denominator) const noexcept
		{
			const auto P = numerator / denominator;
			if (P < 0.0)
				return 0.0;
			return P;
		}
		/// <summary>
		/// Main func for use. NOTE cartesian Y is inverted HERE due to thumbstick hardware. This
		///	might change when a cross-platform build is implemented.
		///	</summary>
		///	<returns>pair X,Y Delay in US</returns>
		[[nodiscard]] auto GetDelaysFromThumbstickValues(const int cartesianX, const int cartesianY)
			const noexcept -> std::pair<size_t, size_t>
		{
			return BuildDelayInfo(cartesianX, -cartesianY);
		}
		///<summary> Calculates microsecond delay values from cartesian X and Y thumbstick values.
		///Probably needs optimized.</summary>
		[[nodiscard]] auto BuildDelayInfo(const auto cartesianX, const auto cartesianY)
		const noexcept -> std::pair<size_t, size_t>
		{
			// If someone ever modifies this again, it is important to remember to scale the result of
			// the polar calculations (based on config file) AFTER translation to delay values.
			// Correction, scale the polar radius FIRST and perform the polar X,Y to delay values calc.
			// And if that doesn't work, I'm going to try scaling the microsecond delay range.
			// Correction, scale the hardware input values.
			using Utilities::ToA;
			using Utilities::ConstAbs;
			// Get info pack for polar computations
			const PolarCalc::PolarCompleteInfoPack fullInfo = m_pc.ComputePolarCompleteInfo(cartesianX, cartesianY);
			// Get scaling mult and inverse
			const auto mult = GetScalingMultiplier(fullInfo.polar_info.polar_theta_angle);
			const auto invMult = GetInverseOfPercentagePlusOne(mult) * AdditionalMultiplier;
			// Apply scale to cartesian values
			const auto scaledX = ToA<int>(cartesianX * invMult);
			const auto scaledY = ToA<int>(cartesianY * invMult);
			// Convert values to delays
			return ConvertToDelays(scaledX, scaledY);
		}
		///<summary> Converts polar magnitudes to delay values. Does not apply any scaling, direct linear interpolation of the inverse percentage. </summary>
		[[nodiscard]] auto ConvertToDelays(const double xScaledValue, const double yScaledValue)
			const noexcept -> std::pair<size_t, size_t>
		{
			using Utilities::ToA;
			using Utilities::ConstAbs;
			// Alias some settings
			const PRadInt ThumbstickValueMax = m_mouse_settings.ThumbstickValueMax+2;
			const auto UsDelayMin = m_mouse_settings.MICROSECONDS_MIN;
			const auto UsDelayMax = m_mouse_settings.MICROSECONDS_MAX;
			// Abs val
			const auto absPolarMagX = ConstAbs(xScaledValue);
			const auto absPolarMagY = ConstAbs(yScaledValue);
			// Get inverse percentages of value to thumbstick max
			const auto xPercent = GetInverseOfPercentage(GetPercentage(absPolarMagX, ThumbstickValueMax));
			const auto yPercent = GetInverseOfPercentage(GetPercentage(absPolarMagY, ThumbstickValueMax));

			const auto clampedX = std::clamp(xPercent, 0.0, 1.0);
			const auto clampedY = std::clamp(yPercent, 0.0, 1.0);
			// Linear interpolation into range of delay min and delay max
			const auto xResult = ToA<size_t>(std::lerp(UsDelayMin, UsDelayMax, clampedX));
			const auto yResult = ToA<size_t>(std::lerp(UsDelayMin, UsDelayMax, clampedY));
			return { xResult, yResult };
		}
		///<summary> Gets the (config file loaded) scaling value for the given (float representation) polarThetaAngle. </summary>
		[[nodiscard]] ScaleFloat GetScalingMultiplier(const std::floating_point auto polarThetaAngle) const noexcept
		{
			using Utilities::ToA;
			using Utilities::ConstAbs;
			// Theta angle of vertical edge of quadrant 1
			static constexpr auto MaxTheta = ToA<unsigned>((std::numbers::pi / 2.0) * 100u);
			// Polar theta angle * 100 converted to an integer value, index into the scaling value array
			auto thetaIndex = ToA<unsigned>(ConstAbs(polarThetaAngle) * 100.0f);
			// Convert from quadrant 2 or 3 to 1 or 4
			if (thetaIndex >= MaxTheta)
				thetaIndex -= MaxTheta;
			// Clamp to handle extraordinary input values.
			thetaIndex = std::clamp(thetaIndex, 0u, MaxTheta);
			// return scaling multiplier
			return m_radius_scale_values[thetaIndex];
		}
	};
}


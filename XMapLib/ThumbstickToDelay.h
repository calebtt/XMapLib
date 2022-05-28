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
		const SensInt m_axis_sensitivity;
		const StickMap m_which_stick;
		const MouseSettings &m_mouse_settings;
		const ScaleRange m_radius_scale_values;
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
				return StickMap::RIGHT_STICK;
			return sm;
		}

		/// <summary> Used to validate sensitivity arg value. </summary>
		[[nodiscard]] static constexpr auto ValidateSensitivity(const SensInt si, const MouseSettings &ms)  noexcept
		{
			//range bind sensitivity
			return std::clamp(si, ms.SENSITIVITY_MIN, ms.SENSITIVITY_MAX);
		}
		/// <summary> Validation func for retrieving the polar angle scaling values. </summary>
		[[nodiscard]] static auto ValidateScaleValues() noexcept
		{
			//read radius scale values config file
			auto tempValues = ReadRadiusScaleValues::GetScalingValues();
			if (tempValues.empty())
				Utilities::LogError("Error in ThumbstickToDelay::ThumbstickToDelay(int,MousePlayerInfo,StickMap), failed to read radius scaling values from config file!");
			return tempValues;
		}
	public:
		/// <summary> Constructor for single axis sensitivity and deadzone processing. </summary>
		/// <exception cref="string"> logs text string if MouseSettings values are unusable. </exception>
		/// <param name="sensitivity">int sensitivity value</param>
		/// <param name="player">MousePlayerInfo struct full of deadzone information</param>
		/// <param name="whichStick">StickMap enum denoting which thumbstick</param>
		///	<param name="ms">Mouse Settings struct containing some default values</param>
		ThumbstickToDelay(
			const int sensitivity, 
			const StickMap whichStick,
			const MouseSettings &ms = {}) noexcept
		: m_axis_sensitivity(ValidateSensitivity(sensitivity, ms)),
		m_which_stick(ValidateStickMap(whichStick)),
		m_mouse_settings(ms),
		m_radius_scale_values(ValidateScaleValues()),
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

		/// <summary>Main func for use. NOTE cartesian Y is inverted HERE due to thumbstick hardware. This
		/// might change when a cross-platform build is implemented. </summary>
		///	<returns>pair X,Y Delay in US</returns>
		[[nodiscard]] auto GetDelaysFromThumbstickValues(const int cartesianX, const int cartesianY)
		const noexcept -> std::pair<size_t,size_t>
		{
			//TODO remember to scale the polar radius before separating into distinct X and Y components
			return BuildDelayInfo(cartesianX, -cartesianY);
		}

		///<summary> Calculates microsecond delay values from cartesian X and Y thumbstick values.
		///Probably needs optimized.</summary>
		[[nodiscard]] auto BuildDelayInfo(const auto cartesianX, const auto cartesianY)
		const -> std::pair<size_t, size_t>
		{
			// If someone ever modifies this again, it is important to remember to scale the result of
			// the polar calculations (based on config file) AFTER translation to delay values.
			// Correction, scale the polar radius FIRST and perform the polar X,Y to delay values calc.
			// And if that doesn't work, I'm going to try scaling the microsecond delay range.
			using Utilities::ToA;
			using Utilities::ConstAbs;
			// Get info pack for polar computations
			PolarCalc::PolarCompleteInfoPack fullInfo{};
			// Get scaled polar magnitudes from cartesian thumb values.
			const auto [xPolarDelay, yPolarDelay] = GetPolarInfo(cartesianX, cartesianY, fullInfo);
			// Scale delay values based on config.
			return ConvertMagnitudesToDelays(xPolarDelay, yPolarDelay);
		}

		///<summary> Computes axis polar radii from cartesian coordinates. Applies scaling based on config file. </summary>
		[[nodiscard]] auto GetPolarInfo(const auto cartesianX, const auto cartesianY, PolarCalc::PolarCompleteInfoPack &fullInfo)
		const noexcept -> PolarCalc::AdjustedMagnitudePack
		{
			using Utilities::ToA;
			// Get polar X and Y magnitudes, and theta angle from cartesian X and Y
			fullInfo = m_pc.ComputePolarCompleteInfo(cartesianX, cartesianY);
			//get scaling mult for theta
			const auto scalingMultiplier = ToA<float>(GetScalingMultiplier(fullInfo.polar_info.polar_theta_angle));
			//here, applying scaling value to polar radius before extraction into separate components
			fullInfo.polar_info.polar_radius *= scalingMultiplier;
			return m_pc.ComputeAdjustedMagnitudes(fullInfo.polar_info, fullInfo.quadrant_info );
		}

		///<summary> Gets the (config file loaded) scaling value for the given polarThetaAngle. </summary>
		[[nodiscard]] auto GetScalingMultiplier(const float polarThetaAngle) const noexcept
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

		///<summary> Converts polar magnitudes to delay values. Does not apply any scaling, direct linear interpolation of the inverse percentage. </summary>
		[[nodiscard]] auto ConvertMagnitudesToDelays(const auto xPolarMag, const auto yPolarMag)
		const noexcept -> std::pair<size_t, size_t>
		{
			using Utilities::ToA;
			using Utilities::ConstAbs;
			// Alias some settings
			constexpr PRadInt PolarRadiusMax = m_mouse_settings.PolarRadiusValueMax;
			constexpr auto UsDelayMin = m_mouse_settings.MICROSECONDS_MIN;
			constexpr auto UsDelayMax = m_mouse_settings.MICROSECONDS_MAX;
			// Clamp to range
			const auto clampedPolarMagX = std::clamp(ToA<double>(xPolarMag), 0.0, ToA<double>(PolarRadiusMax));
			const auto clampedPolarMagY = std::clamp(ToA<double>(yPolarMag), 0.0, ToA<double>(PolarRadiusMax));
			// Lerp inverse percentage to delay values
			const auto xPercent = 1.0 - ToA<double>(clampedPolarMagX / PolarRadiusMax);
			const auto yPercent = 1.0 - ToA<double>(clampedPolarMagY / PolarRadiusMax);
			const auto xResult = ToA<size_t>(std::lerp(UsDelayMin, UsDelayMax, xPercent));
			const auto yResult = ToA<size_t>(std::lerp(UsDelayMin, UsDelayMax, yPercent));
			return { xResult, yResult };
		}
	};
}


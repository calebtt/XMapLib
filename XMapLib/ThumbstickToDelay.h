#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include "ReadRadiusScaleValues.h"
#include "../PolarCode/PolarQuadrantCalc/PolarCalcFaster.h"


namespace sds
{
	/// <summary><c>ThumbstickToDelay</c> Basic logic for mapping thumbstick values to work thread delay values.
	/// This class must be re-instantiated to use new deadzone values.
	/// On the horizon is TMP precomputing each combination of X,Y
	/// that would yield a distinct result.
	///	<para> TODO implement the new sensitivity value modifications
	///	</para>
	/// </summary>
	///	<remarks>A single instance for both thumbstick axes is to be used.</remarks>
	template<class LogFnType = std::function<void(std::string)>>
	class ThumbstickToDelay
	{
		using PolarCalc_t = PolarCalcFaster;
		using DelayType = std::size_t;
		using MultFloat = decltype(MouseSettings::ALT_DEADZONE_MULT_DEFAULT);
		using SensInt = decltype(MouseSettings::SENSITIVITY_DEFAULT);
		using DzInt = decltype(MouseSettings::DEADZONE_DEFAULT);
		using PRadInt = decltype(MouseSettings::PolarRadiusValueMax);
		using ScaleFloat = ReadRadiusScaleValues::FloatType;
		using ScaleRange = ReadRadiusScaleValues::RangeType;
	public:
		void SetSensitivity(const SensInt newSens) noexcept
		{
			m_axis_sensitivity = ValidateSensitivity(newSens, m_mouse_settings);
		}
		[[nodiscard]] SensInt GetSensitivity() const noexcept
		{
			return m_axis_sensitivity;
		}
		void SetStick(const StickMap newStick) noexcept
		{
			m_which_stick = newStick;
		}
		[[nodiscard]] StickMap GetStick() const noexcept
		{
			return m_which_stick;
		}
	private:
		// Additional multiplier applied to polar radius scaling values for cartesian adjustments.
		static constexpr double AdditionalMultiplier{ 1.36 };
		// Axis sensitivity, 1-100
		std::atomic<int> m_axis_sensitivity{};
		// Denotes left stick, right stick, or neither.
		std::atomic<StickMap> m_which_stick;
		// local copy of MouseSettings info struct, contains some program information occasionally used.
		const MouseSettingsPack m_mouse_settings;
		// logging function pointer, used if not null.
		const LogFnType m_logFn;
		// container/range holding radius scaling values.
		const ScaleRange m_radius_scale_values;
		// Utility class for performing polar coordinate calculations.
		PolarCalc_t m_pc;

		///<summary> Used to make some assertions about the settings values this class depends upon. </summary>
		static void AssertSettings(const MouseSettingsPack &ms) noexcept
		{
			//Assertions about the settings values used by this class.
			static_assert(ms.settings.MICROSECONDS_MIN < ms.settings.MICROSECONDS_MAX, 
				"Exception in ThumbstickToDelay() ctor, MICROSECONDS_MIN >= MICROSECONDS_MAX");
			static_assert(ms.settings.SENSITIVITY_MIN < ms.settings.SENSITIVITY_MAX,
				"Exception in ThumbstickToDelay() ctor, SENSITIVITY_MIN >= SENSITIVITY_MAX");
			static_assert(ms.settings.SENSITIVITY_MIN > 0,
				"Exception in ThumbstickToDelay() ctor, SENSITIVITY_MIN <= 0");
			static_assert(ms.settings.SENSITIVITY_MAX <= 100,
				"Exception in ThumbstickToDelay() ctor, SENSITIVITY_MAX > 100");
		}
		/// <summary> Validates StickMap ctor args, neither stick is not a valid setting
		///	for this class, it will default to right stick processing when used. </summary>
		[[nodiscard]] static auto ValidateStickMap(const StickMap sm, const LogFnType logFn)  noexcept
		{
			//error checking StickMap stick setting
			if (sm == StickMap::NEITHER_STICK)
			{
				if(logFn != nullptr)
					logFn("Error in ThumbstickToDelay::ValidateStickMap(StickMap, LogFnType): NEITHER_STICK selected stick, choosing RIGHT_STICK.");
				return StickMap::RIGHT_STICK;
			}
			return sm;
		}
		/// <summary> Used to validate sensitivity arg value to within range SENSITIVITY_MIN to SENSITIVITY_MAX in MouseSettings. </summary>
		[[nodiscard]] static constexpr auto ValidateSensitivity(const SensInt si, const MouseSettingsPack &ms)  noexcept -> int
		{
			//range bind sensitivity
			return std::clamp(si, ms.settings.SENSITIVITY_MIN, ms.settings.SENSITIVITY_MAX);
		}
		/// <summary> Validation func for retrieving the polar angle scaling values from config file. </summary>
		[[nodiscard]] static auto ValidateScaleValues(const MouseSettingsPack &ms, const LogFnType logFn) noexcept
		{
			//read radius scale values config file
			ReadRadiusScaleValues rrsv(ms.settings);
			auto tempValues = rrsv.GetScalingValues();
			if (tempValues.empty() && logFn != nullptr)
			{
				logFn("Error in ThumbstickToDelay::ValidateScaleValues(MouseSettingsPack, LogFnType), failed to read radius scaling values from config file!");
			}
			return tempValues;
		}
	public:
		/// <summary> Constructor for single axis sensitivity and deadzone processing. </summary>
		/// <exception cref="string"> logs text string if MouseSettings values are unusable. </exception>
		/// <param name="sensitivity">int sensitivity value</param>
		/// <param name="whichStick">StickMap enum denoting which thumbstick</param>
		///	<param name="ms">Mouse Settings struct containing some default values</param>
		///	<param name="logFn">Logging function, called to report an error with a C string message.</param>
		ThumbstickToDelay(
			const int sensitivity, 
			const StickMap whichStick,
			const MouseSettingsPack &ms = {},
			const LogFnType logFn = nullptr) noexcept
		: m_axis_sensitivity(ValidateSensitivity(sensitivity, ms)),
		m_which_stick(ValidateStickMap(whichStick, logFn)),
		m_mouse_settings(ms),
		m_logFn(logFn),
		m_radius_scale_values(ValidateScaleValues(ms, logFn)),
		m_pc(ms.settings.PolarRadiusValueMax, logFn)
		{
			AssertSettings(ms);
		}
		ThumbstickToDelay() = delete;
		ThumbstickToDelay(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay(ThumbstickToDelay&& other) = delete;
		ThumbstickToDelay& operator=(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay& operator=(ThumbstickToDelay&& other) = delete;
		~ThumbstickToDelay() = default;

		/// <summary>
		/// Main func for use. NOTE cartesian Y is inverted HERE due to thumbstick hardware. This
		///	might change when a cross-platform build is implemented.
		///	</summary>
		///	<returns>pair X,Y Delay in US</returns>
		[[nodiscard]] auto GetDelaysFromThumbstickValues(const int cartesianX, const int cartesianY)
			const noexcept -> std::pair<DelayType, DelayType>
		{
			if(m_which_stick != StickMap::NEITHER_STICK)
				return BuildDelayInfo(cartesianX, -cartesianY);
			return { m_mouse_settings.settings.MICROSECONDS_MAX, m_mouse_settings.settings.MICROSECONDS_MAX };
		}
		///<summary> Calculates microsecond delay values from cartesian X and Y thumbstick values.
		///Probably needs optimized.</summary>
		[[nodiscard]] auto BuildDelayInfo(const auto cartesianX, const auto cartesianY)
		const noexcept -> std::pair<DelayType, DelayType>
		{
			using namespace sds::Utilities;
			// If someone ever modifies this again, it is important to remember to scale the result of
			// the polar calculations (based on config file) AFTER translation to delay values.
			// Correction, scale the polar radius FIRST and perform the polar X,Y to delay values calc.
			// And if that doesn't work, I'm going to try scaling the microsecond delay range.
			// Correction, scale the hardware input values.
			using Utilities::ToA;
			using Utilities::ConstAbs;
			// Get info pack for polar computations
			const auto fullInfo = m_pc.ComputePolarCompleteInfo(cartesianX, cartesianY);
			// Get scaling mult and inverse
			const auto mult = GetScalingMultiplier(fullInfo.polar_info.polar_theta_angle);
			const auto invMult = GetInverseOfPercentagePlusOne(mult) * AdditionalMultiplier;
			// Apply scale to cartesian values
			const auto scaledX = ToA<int>(cartesianX * invMult);
			const auto scaledY = ToA<int>(cartesianY * invMult);
			// Convert values to delays
			return ConvertToDelays(scaledX, scaledY);
		}
		///<summary> Converts scaled thumbstick values to delay values. Does not apply any scaling, direct linear interpolation of the inverse percentage. </summary>
		[[nodiscard]] auto ConvertToDelays(const double xScaledValue, const double yScaledValue)
			const noexcept -> std::pair<DelayType, DelayType>
		{
			using namespace sds::Utilities;
			// Alias some settings
			const PRadInt ThumbstickValueMax{ m_mouse_settings.settings.ThumbstickValueMax + 2 };
			const auto UsDelayMin = m_mouse_settings.settings.MICROSECONDS_MIN;
			const auto UsDelayMax = m_mouse_settings.settings.MICROSECONDS_MAX;
			// Abs val
			const auto absCartesianScaledX = ConstAbs(xScaledValue);
			const auto absCartesianScaledY = ConstAbs(yScaledValue);
			// Get inverse percentages of value to thumbstick max
			const auto xPercent = GetInverseOfPercentage(GetPercentage(absCartesianScaledX, ThumbstickValueMax));
			const auto yPercent = GetInverseOfPercentage(GetPercentage(absCartesianScaledY, ThumbstickValueMax));

			const auto clampedX = std::clamp(xPercent, 0.0, 1.0);
			const auto clampedY = std::clamp(yPercent, 0.0, 1.0);
			// Linear interpolation into range of delay min and delay max
			const auto xResult = ToA<DelayType>(std::lerp(UsDelayMin, UsDelayMax, clampedX));
			const auto yResult = ToA<DelayType>(std::lerp(UsDelayMin, UsDelayMax, clampedY));
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


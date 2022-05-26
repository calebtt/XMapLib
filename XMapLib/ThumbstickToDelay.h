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
		using MultFloat = float;
		using SensInt = int;
		using DzInt = int;
		using ScaleFloat = ReadRadiusScaleValues::FloatType;
		using ScaleRange = ReadRadiusScaleValues::RangeType;
	public:
	private:
		bool m_is_deadzone_activated{ false };
		const MousePlayerInfo& m_player_info;
		const SensInt m_axis_sensitivity;
		const StickMap m_which_stick;
		const MultFloat m_alt_deadzone_multiplier;
		const DzInt m_polar_magnitude_deadzone;
		const ScaleRange m_radius_scale_values;

		///<summary> Used to make some assertions about the settings values this class depends upon. </summary>
		static void AssertSettings()
		{
			//Assertions about the settings values used by this class.
			if constexpr (MouseSettings::MICROSECONDS_MIN >= MouseSettings::MICROSECONDS_MAX)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, MICROSECONDS_MIN >= MICROSECONDS_MAX");
			if constexpr (MouseSettings::SENSITIVITY_MIN >= MouseSettings::SENSITIVITY_MAX)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MIN >= SENSITIVITY_MAX");
			if constexpr (MouseSettings::SENSITIVITY_MIN <= 0)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MIN <= 0");
			if constexpr (MouseSettings::SENSITIVITY_MAX > 100)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MAX > 100");
		}
		/// <summary> Validates StickMap ctor args, neither stick is not a valid setting
		///	for this class, it will default to right stick processing when used. </summary>
		[[nodiscard]] constexpr auto ValidateStickMap(const StickMap sm) const noexcept
		{
			//error checking StickMap stick setting
			if (sm == StickMap::NEITHER_STICK)
				return StickMap::RIGHT_STICK;
			return sm;
		}
		/// <summary> Used to validate alt deadzone multiplier arg. </summary>
		[[nodiscard]] constexpr auto ValidateAltMultiplier(const MultFloat mf) const noexcept
		{
			return MouseSettings::ALT_DEADZONE_MULT_DEFAULT;
		}
		/// <summary> Used to validate sensitivity arg value. </summary>
		[[nodiscard]] constexpr auto ValidateSensitivity(const SensInt si) const noexcept
		{
			//range bind sensitivity
			return std::clamp(si, MouseSettings::SENSITIVITY_MIN, MouseSettings::SENSITIVITY_MAX);
		}
		/// <summary> Used to validate polar deadzone arg value. </summary>
		[[nodiscard]] constexpr auto ValidatePolarDz(const StickMap sm, const MousePlayerInfo &mpi) const noexcept
		{
			//error checking deadzone arg range, because it might crash the program if the
			//delay returned is some silly value
			const int cdz = sm == StickMap::LEFT_STICK ? mpi.left_polar_dz : mpi.right_polar_dz;
			if (MouseSettings::IsValidDeadzoneValue(cdz))
				return cdz;
			return MouseSettings::DEADZONE_DEFAULT;
		}
		/// <summary> Validation func for retrieving the polar angle scaling values. </summary>
		[[nodiscard]] auto ValidateScaleValues() const noexcept
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
		ThumbstickToDelay(const int sensitivity, const MousePlayerInfo &player, const StickMap whichStick) noexcept
		: m_player_info(player),
		m_axis_sensitivity(ValidateSensitivity(sensitivity)),
		m_which_stick(ValidateStickMap(whichStick)),
		m_alt_deadzone_multiplier(ValidateAltMultiplier(0)),
		m_polar_magnitude_deadzone(ValidatePolarDz(m_which_stick, player)),
		m_radius_scale_values(ValidateScaleValues())
		{
			AssertSettings();
		}
		ThumbstickToDelay() = delete;
		ThumbstickToDelay(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay(ThumbstickToDelay&& other) = delete;
		ThumbstickToDelay& operator=(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay& operator=(ThumbstickToDelay&& other) = delete;
		~ThumbstickToDelay() = default;
		/// <summary>Main func for use.</summary>
		///	<returns>pair X,Y Delay in US</returns>
		[[nodiscard]] auto GetDelaysFromThumbstickValues(const int cartesianX, const int cartesianY) const noexcept
		{
			return BuildDelayInfo(cartesianX, cartesianY, MouseSettings::MICROSECONDS_MIN, MouseSettings::MICROSECONDS_MAX);
		}
		///<summary> Returns the polar rad dz, or the alternate if the dz is already activated.</summary>
		[[nodiscard]] constexpr int GetDeadzoneCurrentValue() const noexcept
		{
			using sds::Utilities::ToA;
			if (m_is_deadzone_activated)
				return ToA<int>(ToA<float>(m_polar_magnitude_deadzone) * m_alt_deadzone_multiplier);
			return m_polar_magnitude_deadzone;
		}
		///<summary> Calculates microsecond delay values from cartesian X and Y thumbstick values.
		///Probably needs optimized.</summary>
		auto BuildDelayInfo(
			const auto cartesianX,
			const auto cartesianY,
			const double us_delay_min,
			const double us_delay_max) const noexcept -> std::pair<std::size_t, std::size_t>
		{
			//TODO this should only use the range between polar_deadzone and polarradiusmax, also there exists a bug lowering the movement speed
			//TODO should make the sensitivity function change the microsec delay minimum, perhaps
			using Utilities::ToA;
			using Utilities::ConstAbs;
			static constexpr double SensMax{ 100.0 };
			static constexpr double PolarRadiusMax{ MouseSettings::ThumbstickValueMax };
			constexpr std::string_view ErrBadAngle = "Error in ThumbstickToDelay::BuildDelayInfo(), fixed theta angle out of bounds.";
			PolarCalc pc(MouseSettings::ThumbstickValueMax, Utilities::LogError);
			//get polar X and Y magnitudes from cartesian X and Y, theta angle, and quadrant number.
			auto fullInfo = pc.ComputePolarCompleteInfo(cartesianX, -cartesianY);
			auto &[xPolarMag, yPolarMag] = fullInfo.adjusted_magnitudes;
			const auto& quadrantNumber = fullInfo.quadrant_info.quadrant_number;
			//use stored scaling values to convert polar radii
			const auto fixedAngle = ToA<unsigned>(ConstAbs(fullInfo.polar_info.polar_theta_angle) * 100.0);
			
			if (fixedAngle < m_radius_scale_values.size() && fixedAngle > 0)
			{
				xPolarMag *= m_radius_scale_values[fixedAngle];
				yPolarMag *= m_radius_scale_values[fixedAngle];
				//std::cout << "Fixed Angle: " << fixedAngle << " Fixed X: " << xPolarMag << " Fixed Y: " << yPolarMag << '\n';
			}
			else
			{
				std::stringstream ss;
				ss << ErrBadAngle << " Angle:[";
				ss << fixedAngle << "]\n";
				Utilities::LogError(ss.str());
			}
			//make sure this is returning good delay values. Apply scaling factors to all quadrants.
			const double percentageX = (xPolarMag / PolarRadiusMax);
			const double percentageY = (yPolarMag / PolarRadiusMax);

			//const auto inverseX = 1.0 / percentageX;
			const auto inverseX = 1.0 - (percentageX);
			//const auto inverseY = 1.0 / percentageY;
			const auto inverseY = 1.0 - (percentageY);
			//std::cout << "inverseX: " << inverseX << '\n';
			//std::cout << "inverseY: " << inverseY << '\n';
			const auto interpX = std::lerp(us_delay_min, us_delay_max, inverseX);
			const auto interpY = std::lerp(us_delay_min, us_delay_max, inverseY);
			//std::cout << "interpX: " << interpX << '\n';
			//std::cout << "interpY: " << interpY << '\n';
			return std::make_pair<size_t,size_t>(ToA<size_t>(interpX), ToA<size_t>(interpY));
			//const auto inverseX = us_delay_max - (integralPercentageX * step + us_delay_min);
			//const auto inverseY = us_delay_max - (integralPercentageY * step + us_delay_min);
			//return std::make_pair<size_t, size_t>(ToA<int>(std::clamp(inverseX, us_delay_min, us_delay_max)), ToA<int>(std::clamp(inverseY, us_delay_min, us_delay_max)));
		}
		///<summary>Takes a cartesian value and returns true if equal or over deadzone. </summary>
		[[nodiscard]] bool IsBeyondDeadzone(const int cartesianThumbstickValue) const noexcept
		{
			using Utilities::ToA;
			using Utilities::ConstAbs;
			return ConstAbs(cartesianThumbstickValue) >= GetDeadzoneCurrentValue();
		}
	};
}


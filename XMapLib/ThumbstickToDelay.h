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
	public:
	private:
		bool m_is_deadzone_activated{ false };
		float m_alt_deadzone_multiplier{ MouseSettings::ALT_DEADZONE_MULT_DEFAULT };
		int m_axis_sensitivity{ MouseSettings::SENSITIVITY_DEFAULT };
		int m_polar_magnitude_deadzone{ MouseSettings::DEADZONE_DEFAULT };
		MousePlayerInfo m_player_info{};
		StickMap m_which_stick;
		std::vector<double> m_radius_scale_values;
		//Used to make some assertions about the settings values this class depends upon.
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
		//Returns the polar rad dz, or the alternate if the dz is already activated.
		[[nodiscard]] int GetDeadzoneCurrentValue() const noexcept
		{
			using sds::Utilities::ToA;
			if (m_is_deadzone_activated)
				return ToA<int>(ToA<float>(m_polar_magnitude_deadzone) * m_alt_deadzone_multiplier);
			return m_polar_magnitude_deadzone;
		}
	public:
		/// <summary>Ctor for dual axis sensitivity and deadzone processing.
		/// Allows getting sensitivity values for the current axis, from using alternate deadzones and sensitivity values for each axis.
		/// In effect, the delay values returned will be influenced by the state of the other axis.</summary>
		/// <exception cref="std::string"> logs std::string if XinSettings values are unusable. </exception>
		/// <param name="sensitivity">int sensitivity value</param>
		/// <param name="player">MousePlayerInfo struct full of deadzone information</param>
		/// <param name="whichStick">StickMap enum denoting which thumbstick</param>
		ThumbstickToDelay(const int sensitivity, const MousePlayerInfo &player, const StickMap whichStick) noexcept
		: m_player_info(player), m_which_stick(whichStick)
		{
			AssertSettings();
			//error checking mousemap stick setting
			if (m_which_stick == StickMap::NEITHER_STICK)
				m_which_stick = StickMap::RIGHT_STICK;
			//range bind sensitivity
			m_axis_sensitivity = std::clamp(sensitivity, MouseSettings::SENSITIVITY_MIN, MouseSettings::SENSITIVITY_MAX);
			//error checking deadzone arg range, because it might crash the program if the
			//delay returned is some silly value
			const int cdz = m_which_stick == StickMap::LEFT_STICK ? player.left_polar_dz : player.right_polar_dz;
			if (MouseSettings::IsValidDeadzoneValue(cdz))
				m_polar_magnitude_deadzone = cdz;
			//read radius scale values config file
			m_radius_scale_values = ReadRadiusScaleValues::GetScalingValues();
		}
		ThumbstickToDelay() = delete;
		ThumbstickToDelay(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay(ThumbstickToDelay&& other) = delete;
		ThumbstickToDelay& operator=(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay& operator=(ThumbstickToDelay&& other) = delete;
		~ThumbstickToDelay() = default;
		/// <summary>Main func for use.</summary>
		///	<returns>pair X,Y Delay in US</returns>
		[[nodiscard]] auto GetDelaysFromThumbstickValues(const int cartesianX, const int cartesianY) const noexcept -> std::pair<std::size_t,std::size_t>
		{
			const auto delayPair = BuildDelayInfo(cartesianX, cartesianY, MouseSettings::MICROSECONDS_MIN, MouseSettings::MICROSECONDS_MAX);
			return delayPair;
		}
		///<summary> Calculates microsecond delay values from cartesian X and Y thumbstick values.
		///Probably needs optimized.</summary>
		auto BuildDelayInfo(
			const auto cartesianX,
			const auto cartesianY,
			const double us_delay_min,
			const double us_delay_max) const noexcept
		{
			//TODO this should only use the range between polar_deadzone and polarradiusmax, also there exists a bug lowering the movement speed
			//TODO should make the sensitivity function change the microsec delay minimum, perhaps
			using sds::Utilities::ToA;
			static constexpr double SensMax{ 100.0 };
			static constexpr double PolarRadiusMax{ MouseSettings::ThumbstickValueMax };
			PolarCalc pc(MouseSettings::ThumbstickValueMax, Utilities::LogError);
			//get some polar info
			auto fullInfo = pc.ComputePolarCompleteInfo(cartesianX, cartesianY);
			auto &[xPolarMag, yPolarMag] = fullInfo.adjusted_magnitudes;

			//use stored scaling values to convert polar radii
			const unsigned fixedAngle = ToA<unsigned>(std::abs(fullInfo.polar_info.polar_theta_angle * 100.0));
			if (fixedAngle < 0)
				Utilities::LogError("Error in ThumbstickToDelay::BuildDelayInfo(), fixed theta angle out of bounds.");
			if (fixedAngle < m_radius_scale_values.size() && fixedAngle > 0)
			{
				xPolarMag *= m_radius_scale_values[fixedAngle];
				yPolarMag *= m_radius_scale_values[fixedAngle];
			}
			//TODO, add reading the scaling values file using readradiusscalevalues
			//and make sure this is returning good delay values. Apply scaling factors to all quadrants.

			const double percentageX = (xPolarMag / PolarRadiusMax);
			const double percentageY = (yPolarMag / PolarRadiusMax);

			//const auto inverseX = 1.0 / percentageX;
			const auto inverseX = 1.0 - (percentageX);
			//const auto inverseY = 1.0 / percentageY;
			const auto inverseY = 1.0 - (percentageY);
			std::cout << "inverseX: " << inverseX << '\n';
			std::cout << "inverseY: " << inverseY << '\n';
			const auto interpX = std::lerp(us_delay_min, us_delay_max, inverseX);
			const auto interpY = std::lerp(us_delay_min, us_delay_max, inverseY);
			std::cout << "interpX: " << interpX << '\n';
			std::cout << "interpY: " << interpY << '\n';
			return std::make_pair<size_t,size_t>(ToA<size_t>(interpX), ToA<size_t>(interpY));
			//const auto inverseX = us_delay_max - (integralPercentageX * step + us_delay_min);
			//const auto inverseY = us_delay_max - (integralPercentageY * step + us_delay_min);
			//return std::make_pair<size_t, size_t>(ToA<int>(std::clamp(inverseX, us_delay_min, us_delay_max)), ToA<int>(std::clamp(inverseY, us_delay_min, us_delay_max)));
		}
		///<summary>Takes a cartesian value and returns true if equal or over deadzone. </summary>
		[[nodiscard]] bool IsBeyondDeadzone(const int cartesianThumbstickValue) const noexcept
		{
			using sds::Utilities::ToA;
			using sds::Utilities::ConstAbs;
			return ConstAbs(cartesianThumbstickValue) >= GetDeadzoneCurrentValue();
		}
	};
}


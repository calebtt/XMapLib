#pragma once
#include "stdafx.h"
#include "SensitivityMapper.h"
#include "Utilities.h"

namespace sds
{
	/// <summary>Basic logic for mapping thumbstick values to work thread delay values.
	/// A single instance for a single thumbstick axis is to be used.
	/// This class must be re-instantiated to use new deadzone values.</summary>
	class ThumbstickToDelay
	{
	public:
		using SensMapType = std::map<int, int>;
	private:
		const std::string BAD_DELAY_MSG{ "Bad timer delay value, exception." };
		inline static std::atomic<bool> m_is_deadzone_activated{ false }; //shared between instances
		float m_alt_deadzone_multiplier{ MouseSettings::ALT_DEADZONE_MULT_DEFAULT };
		int m_axis_sensitivity{ MouseSettings::SENSITIVITY_DEFAULT };
		int m_x_axis_deadzone{ MouseSettings::DEADZONE_DEFAULT };
		int m_y_axis_deadzone{ MouseSettings::DEADZONE_DEFAULT };
		SensitivityMapper m_sensitivity_mapper{};
		SensMapType m_shared_sensitivity_map{};
		const bool m_is_x_axis;
		//Used to make some assertions about the settings values this class depends upon.
		static void AssertSettings()
		{
			//Assertions about the settings values used by this class.
			if constexpr (MouseSettings::MICROSECONDS_MIN_MAX >= MouseSettings::MICROSECONDS_MAX)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, MICROSECONDS_MIN_MAX >= MICROSECONDS_MAX");
			if constexpr (MouseSettings::MICROSECONDS_MIN_MAX <= MouseSettings::MICROSECONDS_MIN)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, MICROSECONDS_MIN_MAX <= MICROSECONDS_MIN");
			if constexpr (MouseSettings::MICROSECONDS_MIN >= MouseSettings::MICROSECONDS_MAX)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, MICROSECONDS_MIN >= MICROSECONDS_MAX");
			if constexpr (MouseSettings::SENSITIVITY_MIN >= MouseSettings::SENSITIVITY_MAX)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MIN >= SENSITIVITY_MAX");
			if constexpr (MouseSettings::SENSITIVITY_MIN <= 0)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MIN <= 0");
			if constexpr (MouseSettings::SENSITIVITY_MAX > 100)
				Utilities::LogError("Exception in ThumbstickToDelay() ctor, SENSITIVITY_MAX > 100");
		}
		void InitFirstPiece(int sensitivity, int xAxisDz, int yAxisDz)
		{
			m_axis_sensitivity = RangeBindValue(sensitivity, MouseSettings::SENSITIVITY_MIN, MouseSettings::SENSITIVITY_MAX);
			if (!MouseSettings::IsValidDeadzoneValue(xAxisDz))
				xAxisDz = MouseSettings::DEADZONE_DEFAULT;
			if (!MouseSettings::IsValidDeadzoneValue(yAxisDz))
				yAxisDz = MouseSettings::DEADZONE_DEFAULT;
			m_x_axis_deadzone = xAxisDz;
			m_y_axis_deadzone = yAxisDz;
		}
		[[nodiscard]] constexpr int RangeBindValue(const int user_sens, const int sens_min, const int sens_max) const noexcept
		{
			//bounds check result
			if (user_sens > sens_max)
				return sens_max;
			else if (user_sens < sens_min)
				return sens_min;
			return user_sens;
		}
		[[nodiscard]] constexpr bool IsBeyondDeadzone(const int val, const bool isX) const noexcept
		{
			using namespace Utilities;
			auto GetDeadzoneCurrent = [this](const bool isItX)
			{
				return ToA<int>(isItX ? this->m_x_axis_deadzone : this->m_y_axis_deadzone);
			};
			const bool move = 
				(ToA<float>(val) > ToA<float>(GetDeadzoneCurrent(isX)) 
					|| ToA<float>(val) < -ToA<float>(GetDeadzoneCurrent(isX)));
			return move;
		}
		[[nodiscard]] constexpr bool IsBeyondAltDeadzone(const int val, const bool isItX) const noexcept
		{
			using namespace Utilities;
			auto GetDeadzoneCurrent = [this](const bool isTheX)
			{
				return ToA<int>(isTheX ? (ToA<float>(m_x_axis_deadzone) * m_alt_deadzone_multiplier) : (ToA<float>(m_y_axis_deadzone) * m_alt_deadzone_multiplier));
			};
			const bool move =
				(ToA<float>(val) > ToA<float>(GetDeadzoneCurrent(isItX))
					|| ToA<float>(val) < -ToA<float>(GetDeadzoneCurrent(isItX)));
			return move;
		}
		//Returns the dz for the axis, or the alternate if the dz is already activated.
		[[nodiscard]] int GetDeadzoneActivated(const bool isX) const noexcept
		{
			using namespace Utilities;
			int dz = 0;
			if (m_is_deadzone_activated)
				dz = ToA<int>(isX ? (ToA<float>(m_x_axis_deadzone) * m_alt_deadzone_multiplier) : (ToA<float>(m_y_axis_deadzone) * m_alt_deadzone_multiplier));
			else
				dz = isX ? m_x_axis_deadzone : m_y_axis_deadzone;
			return dz;
		}
	public:
		/// <summary>Ctor for dual axis sensitivity and deadzone processing.
		/// Allows getting sensitivity values for the current axis, from using alternate deadzones and sensitivity values for each axis.
		/// In effect, the delay values returned will be influenced by the state of the other axis.</summary>
		/// <exception cref="std::string"> logs std::string if XinSettings values are unusable. </exception>
		/// <param name="sensitivity">int sensitivity value</param>
		/// <param name="player">MousePlayerInfo struct full of deadzone information</param>
		/// <param name="whichStick">StickMap enum denoting which thumbstick</param>
		/// <param name="isX">is it for the X axis?</param>
		ThumbstickToDelay(const int sensitivity, const MousePlayerInfo &player, StickMap whichStick, const bool isX) noexcept : m_is_x_axis(isX)
		{
			AssertSettings();
			//error checking mousemap stick setting
			if (whichStick == StickMap::NEITHER_STICK)
				whichStick = StickMap::RIGHT_STICK;
			//error checking axisDeadzone arg range, because it might crash the program if the
			//delay returned is some silly value
			const int cdx = whichStick == StickMap::LEFT_STICK ? player.left_x_dz : player.right_x_dz;
			const int cdy = whichStick == StickMap::LEFT_STICK ? player.left_y_dz : player.right_y_dz;
			InitFirstPiece(sensitivity, cdx, cdy);
			m_shared_sensitivity_map = m_sensitivity_mapper.BuildSensitivityMap(m_axis_sensitivity,
				MouseSettings::SENSITIVITY_MIN,
				MouseSettings::SENSITIVITY_MAX,
				MouseSettings::MICROSECONDS_MIN,
				MouseSettings::MICROSECONDS_MAX,
				MouseSettings::MICROSECONDS_MIN_MAX);
		}
		ThumbstickToDelay() = delete;
		ThumbstickToDelay(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay(ThumbstickToDelay&& other) = delete;
		ThumbstickToDelay& operator=(const ThumbstickToDelay& other) = delete;
		ThumbstickToDelay& operator=(ThumbstickToDelay&& other) = delete;
		~ThumbstickToDelay() = default;
		/// <summary>returns a copy of the internal sensitivity map</summary>
		/// <returns>std map of int, int</returns>
		[[nodiscard]] std::map<int, int> GetCopyOfSensitivityMap() const
		{
			return m_shared_sensitivity_map;
		}
		/// <summary>Determines if m_is_x_axis axis requires move based on alt deadzone if dz is activated.</summary>
		[[nodiscard]] bool DoesAxisRequireMoveAlt(const int x, const int y) const noexcept
		{
			if (!m_is_deadzone_activated)
			{
				const bool xMove = IsBeyondDeadzone(x, true);
				const bool yMove = IsBeyondDeadzone(y, false);
				m_is_deadzone_activated = xMove || yMove;
				return m_is_x_axis ? xMove : yMove;
			}
			else
			{
				const bool xMove = IsBeyondAltDeadzone(x, true);
				const bool yMove = IsBeyondAltDeadzone(y, false);
				m_is_deadzone_activated = xMove || yMove;
				return m_is_x_axis ? xMove : yMove;
			}
		}
		/// <summary>Main func for use.</summary>
		/// <returns>Delay in US</returns>
		[[nodiscard]] size_t GetDelayFromThumbstickValue(int x, int y) const noexcept
		{
			const int xdz = GetDeadzoneActivated(true);
			const int ydz = GetDeadzoneActivated(false);
			x = GetRangedThumbstickValue(x, xdz);
			y = GetRangedThumbstickValue(y, ydz);
			//The transformation function applied to consider the value of both axes in the calculation.
			auto TransformSensitivityValue = [this](const int tx, const int ty, const bool isX)
			{
				using namespace Utilities;
				double txVal = MouseSettings::SENSITIVITY_MIN;
				if (isX && (ty != 0))
					txVal = ToA<double>(ConstAbs(tx)) + (std::sqrt(ConstAbs(ty)) * 3.6);
				else if (tx != 0)
					txVal = ToA<double>(ConstAbs(ty)) + (std::sqrt(ConstAbs(tx)) * 3.6);
				return static_cast<int>(txVal);
			};
			x = TransformSensitivityValue(x, y, true);
			y = TransformSensitivityValue(x, y, false);
			const int txVal = GetMappedValue(m_is_x_axis ? x : y);
			return txVal;
		}
		///<summary>Retrieves value mapped to the key, with error checking. </summary>
		[[nodiscard]] int GetMappedValue(int keyValue) const noexcept
		{
			keyValue = RangeBindValue(keyValue, MouseSettings::SENSITIVITY_MIN, MouseSettings::SENSITIVITY_MAX);
			//error checking to make sure the value is in the map
			if(!m_shared_sensitivity_map.contains(keyValue))
			{
				//this should not happen, but in case it does I want a plain string telling me it did.
				Utilities::LogError("Exception in ThumbstickToDelay::GetDelayFromThumbstickValue(int,int,bool): " + BAD_DELAY_MSG);
				return 1;
			}
			const auto rval = m_shared_sensitivity_map.at(keyValue);
			if(rval >= MouseSettings::MICROSECONDS_MIN && rval <= MouseSettings::MICROSECONDS_MAX)
			{
				return rval;
			}
			else
			{
				Utilities::LogError("ThumbstickToDelay::GetDelayFromThumbstickValue(): Failed to acquire mapped value with key: " + (std::to_string(keyValue)));
				return MouseSettings::MICROSECONDS_MAX;
			}
		}
		/// <summary>For the case where sensitivity range is 1 to 100 this function will convert the thumbstick value to
		/// an integer percentage. The deadzone value is subtracted before processing.</summary>
		/// <param name="thumbstick">thumbstick value between short minimum and short maximum</param>
		/// <param name="axisDeadzone">positive deadzone value to use for the axis value</param>
		/// <returns>positive value between (inclusive) SENSITIVITY_MIN and SENSITIVITY_MAX, or SENSITIVITY_MIN for thumbstick less than deadzone</returns>
		[[nodiscard]] constexpr int GetRangedThumbstickValue(int thumbstick, int axisDeadzone) const noexcept
		{
			using namespace sds::Utilities;
			thumbstick = RangeBindValue(thumbstick, MouseSettings::SMin, MouseSettings::SMax);
			if (thumbstick == 0)
				return MouseSettings::SENSITIVITY_MIN;
			//error checking deadzone arg range
			if (!MouseSettings::IsValidDeadzoneValue(axisDeadzone))
				axisDeadzone = MouseSettings::DEADZONE_DEFAULT;
			const int absThumb = ConstAbs(thumbstick);
			if (absThumb < axisDeadzone)
				return MouseSettings::SENSITIVITY_MIN;
			int percentage = (absThumb - axisDeadzone) / ((MouseSettings::SMax - axisDeadzone) / MouseSettings::SENSITIVITY_MAX);
			percentage = RangeBindValue(percentage, MouseSettings::SENSITIVITY_MIN, MouseSettings::SENSITIVITY_MAX);
			return ToA<int>(percentage);
		}
	};
}


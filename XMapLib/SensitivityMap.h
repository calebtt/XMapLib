#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include <cmath>
#include <numeric>

namespace sds
{
	struct SensitivityMap
	{
	private:
		const std::string m_except_minimum = "Exception in SensitivityMap::SensitivityToMinimum(): ";
		const std::string m_except_build_map = "Exception in SensitivityMap::BuildSensitivityMap(): ";
	public:
		/// <summary>
		/// Builds a sensitivity map that maps values from sens_min to sens_max to values between
		/// us_delay_min and us_delay_max
		/// </summary>
		/// <param name="user_sens">user sensitivity value</param>
		/// <param name="sens_min">minimum sensitivity value</param>
		/// <param name="sens_max">maximum sensitivity value</param>
		/// <param name="us_delay_min">minimum delay in microseconds</param>
		/// <param name="us_delay_max">maximum delay in microseconds</param>
		/// <param name="us_delay_min_max">minimum microsecond delay maximum value, used by the user sensitivity adjustment function</param>
		/// <returns>map int,int mapping sensitivity values to microsecond delay values</returns>
		[[nodiscard]] std::map<int, int> BuildSensitivityMap(const int user_sens, 
			const int sens_min, 
			const int sens_max, 
			const int us_delay_min, 
			const int us_delay_max, 
			const int us_delay_min_max) const noexcept
		{
			using namespace sds::Utilities; // for ToFloat() and ToDub() and LogError()
			//arg error checking
			if (sens_min >= sens_max || us_delay_min >= us_delay_max || user_sens < sens_min || user_sens > sens_max)
				LogError(m_except_build_map + "user sensitivity, or sensitivity range or delay range out of range.");
			auto LogErrorIfFalse = [this](bool val)
			{
				if(!val)
					LogError(m_except_build_map + "computed value not std::isnormal()");
			};
			//getting new minimum using minimum maximum
			const int adjustedMinimum = SensitivityToMinimum(user_sens, sens_min, sens_max, us_delay_min, us_delay_min_max);
			const float fstep = (ToFloat(us_delay_max) - ToFloat(adjustedMinimum)) / (ToFloat(sens_max) - ToFloat(sens_min));
			const int step = static_cast<int>(std::lroundf(fstep));
			LogErrorIfFalse(IsNormalF(adjustedMinimum));
			LogErrorIfFalse(IsNormalF(fstep));
			LogErrorIfFalse(IsNormalF(step));
			std::map<int, int> sens_map;
			for (auto i = sens_min, j = us_delay_max; i <= sens_max; i++, j-=step)
			{
				if (j < adjustedMinimum)
				{
					sens_map[i] = adjustedMinimum;
				}
				else
				{
					sens_map[i] = j;
				}
			}
			return sens_map;
		}

		/// <summary>
		/// Returns the user sensitivity adjusted minimum microsecond delay based
		/// on the arguments. This is used to alter the minimum microsecond delay of the sensitivity map,
		/// when a sensitivity value is used.
		/// </summary>
		/// <param name="user_sens">a user sensitivity value</param>
		/// <param name="sens_min">minimum possible sensitivity value</param>
		/// <param name="sens_max">maximum possible sensitivity value</param>
		/// <param name="us_delay_min">minimum delay in microseconds for the range of minimum delay values</param>
		/// <param name="us_delay_max">maximum delay in microseconds for the range of minimum delay values</param>
		/// <returns>sensitivity adjusted minimum microsecond delay </returns>
		int SensitivityToMinimum(const int user_sens, 
			const int sens_min, 
			const int sens_max, 
			const int us_delay_min, 
			const int us_delay_max) const
		{
			using namespace sds::Utilities;
			//arg error checking
			if (sens_min >= sens_max || us_delay_min >= us_delay_max || user_sens < sens_min || user_sens > sens_max)
			{
				Utilities::LogError(m_except_minimum + "user sensitivity, or sensitivity range or delay range out of range.");
				return 1;
			}
			const double sensitivityRange = ToDub(sens_max) - ToDub(sens_min);
			const double step = (ToDub(us_delay_max) - ToDub(us_delay_min)) / sensitivityRange;
			std::vector<double> delayVec;
			for (auto i = sens_min, j = 0; i <= sens_max; i++, j++)
				delayVec.push_back(ToDub(us_delay_min) + (ToDub(j) * step));
			//adapt user_sens and sensitivityRange to vector indexes
			const int elementIndex = sens_max - user_sens;
			return static_cast<int>(delayVec.at(elementIndex));
		}
	};
}


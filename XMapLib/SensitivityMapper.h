#pragma once
#include "stdafx.h"
#include "Utilities.h"
#include <cmath>
namespace sds
{
	struct SensitivityMapper
	{
	public:
		using SensMapType = std::map<int, int>;
	private:
		const std::string m_except_minimum{ "Exception in SensitivityMapper::SensitivityToMinimum(): " };
		const std::string m_except_build_map{ "Exception in SensitivityMapper::BuildSensitivityMap(): " };
	public:
		/// <summary>Builds a sensitivity map that maps values from sens_min to sens_max to values between us_delay_min and us_delay_max</summary>
		/// <param name="user_sens">user sensitivity value</param>
		/// <param name="sens_min">minimum sensitivity value</param>
		/// <param name="sens_max">maximum sensitivity value</param>
		/// <param name="us_delay_min">minimum delay in microseconds</param>
		/// <param name="us_delay_max">maximum delay in microseconds</param>
		/// <param name="us_delay_min_max">minimum microsecond delay maximum value, used by the user sensitivity adjustment function</param>
		/// <returns>map int,int mapping sensitivity values to microsecond delay values</returns>
		[[nodiscard]] SensMapType BuildSensitivityMap(const int user_sens, 
			const int sens_min, 
			const int sens_max, 
			const int us_delay_min, 
			const int us_delay_max, 
			const int us_delay_min_max) const noexcept
		{
			using namespace sds::Utilities; // for ToA<float>() and ToDub() and LogError()
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
			const float fstep = (ToA<float>(us_delay_max) - ToA<float>(adjustedMinimum)) / (ToA<float>(sens_max) - ToA<float>(sens_min));
			const int step = ToA<int>(std::lroundf(fstep));
			LogErrorIfFalse(IsNormalF(adjustedMinimum));
			LogErrorIfFalse(IsNormalF(fstep));
			LogErrorIfFalse(IsNormalF(step));
			SensMapType sens_map;
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

		/// <summary>Returns the user sensitivity adjusted minimum microsecond delay based
		/// on the arguments. This is used to alter the minimum microsecond delay of the sensitivity map,
		/// when a sensitivity value is used.</summary>
		/// <param name="user_sens">a user sensitivity value</param>
		/// <param name="sens_min">minimum possible sensitivity value</param>
		/// <param name="sens_max">maximum possible sensitivity value</param>
		/// <param name="us_delay_min">minimum delay in microseconds for the range of minimum delay values</param>
		/// <param name="us_delay_max">maximum delay in microseconds for the range of minimum delay values</param>
		/// <returns>sensitivity adjusted minimum microsecond delay </returns>
		[[nodiscard]] int SensitivityToMinimum(const int user_sens, 
			const int sens_min, 
			const int sens_max, 
			const int us_delay_min, 
			const int us_delay_max) const noexcept
		{
			using namespace sds::Utilities;
			//arg error checking
			if (sens_min >= sens_max || us_delay_min >= us_delay_max || user_sens < sens_min || user_sens > sens_max)
			{
				Utilities::LogError(m_except_minimum + "user sensitivity, or sensitivity range or delay range out of range.");
				return 1;
			}
			const double sensitivityRange = ToA<double>(sens_max) - ToA<double>(sens_min);
			const double step = (ToA<double>(us_delay_max) - ToA<double>(us_delay_min)) / sensitivityRange;
			std::vector<double> delayVec;
			for (auto i = sens_min, j = 0; i <= sens_max; i++, j++)
				delayVec.emplace_back(ToA<double>(us_delay_min) + (ToA<double>(j) * step));
			//adapt user_sens and sensitivityRange to vector indexes
			const int elementIndex = sens_max - user_sens;
			return ToA<int>(delayVec[elementIndex]);
		}
	};
}


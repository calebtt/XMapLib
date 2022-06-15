#pragma once
#include "stdafx.h"
#include <ostream>
#include <chrono>

namespace sds::Utilities
{
	/// <summary> <c>DelayManager</c> Enables asynchronously waiting for a microsecond precision timer
	///	to elapse. </summary>
	class DelayManager
	{
		using ClockType = std::chrono::steady_clock;
		using TimeType = std::chrono::time_point<ClockType>;
		TimeType m_start_time{ ClockType::now() };
		size_t m_duration{ 1 };
		bool m_has_fired{ false };
	public:
		//us is microseconds
		DelayManager() = delete;
		explicit DelayManager(size_t duration_us) : m_duration(duration_us) { }
		DelayManager(const DelayManager& other) = default;
		DelayManager(DelayManager&& other) = default;
		DelayManager& operator=(const DelayManager& other) = default;
		DelayManager& operator=(DelayManager&& other) = default;
		~DelayManager() = default;
		/// <summary>Operator<< overload for std::ostream specialization,
		///	writes more detailed delay details for debugging.
		///	Thread-safe, provided all writes to the ostream object
		///	are wrapped with std::osyncstream!</summary>
		friend std::ostream& operator<<(std::ostream& os, const DelayManager& obj) noexcept
		{
			std::osyncstream ss(os);
			ss << "[DelayManager]" << std::endl
				<< "m_start_time:" << obj.m_start_time.time_since_epoch() << std::endl
				<< "m_duration (microseconds):" << obj.m_duration << std::endl
				<< "m_has_fired:" << obj.m_has_fired << std::endl
				<< "[/DelayManager]";
			return os;
		}
		/// <summary>Check for elapsed.</summary>
		bool IsElapsed() noexcept
		{
			if (ClockType::now() > (m_start_time + std::chrono::microseconds(m_duration)))
			{
				m_has_fired = true;
				return true;
			}
			return false;
		}
		/// <summary>Reset delay for elapsing.</summary>
		void Reset(size_t microsec_delay) noexcept
		{
			m_start_time = ClockType::now();
			m_has_fired = false;
			m_duration = microsec_delay;
		}
	};
}

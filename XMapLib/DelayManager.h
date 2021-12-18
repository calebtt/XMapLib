#pragma once
#include <ostream>
#include <chrono>

namespace sds::Utilities
{
	class DelayManager
	{
		using TimeType = std::chrono::time_point<std::chrono::high_resolution_clock>;
		TimeType m_start_time = std::chrono::high_resolution_clock::now();
		size_t m_duration = 1;
		bool m_has_fired = false;
	public:
		//us is microseconds
		DelayManager() = delete;
		DelayManager(size_t duration_us) : m_duration(duration_us) { }
		DelayManager(const DelayManager& other) = default;
		DelayManager(DelayManager&& other) = default;
		DelayManager& operator=(const DelayManager& other) = default;
		DelayManager& operator=(DelayManager&& other) = default;
		~DelayManager() = default;
		friend std::ostream& operator<<(std::ostream& os, const DelayManager& obj)
		{
			return os << "[DelayManager] "
				<< "m_start_time: " << obj.m_start_time.time_since_epoch() << "\n"
				<< "m_duration (microseconds): " << obj.m_duration << "\n"
				<< "m_has_fired: " << obj.m_has_fired
				<< " [/DelayManager]";
		}
		/// <summary>
		/// Check for elapsed.
		/// </summary>
		bool IsElapsed()
		{
			if (std::chrono::high_resolution_clock::now() > (m_start_time + std::chrono::microseconds(m_duration)))
			{
				m_has_fired = true;
				return true;
			}
			return false;
		}
		void Reset(size_t newDuration)
		{
			m_start_time = std::chrono::high_resolution_clock::now();
			m_has_fired = false;
			m_duration = newDuration;
		}
	};
}


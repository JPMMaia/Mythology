#ifndef MAIA_UTILITIES_TIMER_H_INCLUDED
#define MAIA_UTILITIES_TIMER_H_INCLUDED

#include <chrono>
#include <cstddef>

namespace Maia::Utilities
{
	class Timer
	{
	public:

		using clock = std::chrono::high_resolution_clock;
		using time_point = clock::time_point;
		using duration = clock::duration;

		Timer(duration const fixed_update_duration) :
			m_begin_time{ clock::now() },
			m_previous_time{ m_begin_time },
			m_current_time{ m_current_time },
			m_fixed_update_duration{ fixed_update_duration },
			m_frame_counter{ 0 }
		{
		}

		void update_time_counters()
		{
			m_previous_time = m_current_time;
			m_current_time = clock::now();
		}

		void increment_frame_counter()
		{
			++m_frame_counter;
		}

		duration total_time() const
		{
			return m_current_time - m_begin_time;
		}
		duration delta_time() const
		{
			return m_current_time - m_previous_time;
		}
		duration fixed_update_duration() const
		{
			return m_fixed_update_duration;
		}
		std::size_t frame_counter() const
		{
			return m_frame_counter;
		}

	private:

		time_point const m_begin_time;
		time_point m_previous_time;
		time_point m_current_time;
		duration const m_fixed_update_duration;
		std::size_t m_frame_counter;

	};
}

#endif

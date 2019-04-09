import std.core;

export module Maia.Utilities.Timers;

namespace Maia
{
	namespace Utilities
	{
		export class PerformanceTimer
		{
		private:

			// Public definitions:
			using ClockType = std::chrono::high_resolution_clock;
			using TimePointType = ClockType::time_point;

			// Start and end timer:
			void Start()
			{
				m_start = ClockType::now();
			}
			void end()
			{
				m_end = ClockType::now();
			}

			// Getters:
			template <class Representation, class Period>
			std::chrono::duration<Representation, Period> GetElapsedTime() const
			{
				return m_end - m_start;
			}

		private:

			// Members:
			TimePointType m_start;
			TimePointType m_end;

		};
	}
}

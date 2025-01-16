#pragma once

#include "Defines.hpp"
#include "Types.hpp"
#include <chrono>

namespace Luden::Core
{
	// Engine timer
	// Keeps track of current frame per second count
	// and allows to limit max FPS to specific range.
	// Note: need simplification.
	class Timer
	{
	public:
		Timer() = default;
		~Timer() = default;

		void Start()
		{
			const auto now = std::chrono::high_resolution_clock::now();

			m_CurrentTime = now;
			m_StartTime = now;
			m_LastUpdate = now;

			m_StopTime = std::chrono::high_resolution_clock::time_point{};
			m_PausedTime += (m_CurrentTime - m_StopTime);

			bIsStopped = false;
		}

		void Stop()
		{
			m_StopTime = std::chrono::high_resolution_clock::now();

			bIsStopped = true;
		}

		void Tick()
		{
			if (bIsStopped)
			{
				DeltaTime = 0.0;

				return;
			}

			m_CurrentTime = std::chrono::high_resolution_clock::now();

			DeltaTime = std::chrono::duration<f64, std::milli>(m_CurrentTime - m_LastUpdate).count();

			m_LastUpdate = m_CurrentTime;

			if (DeltaTime < 0.0)
			{
				DeltaTime = 0.0f;
			}
		}

		f64 TotalTime() const
		{
			auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(m_CurrentTime - m_StartTime);

			return static_cast<f64>(totalTime.count());
		}

		void GetFrameStats()
		{
			++FrameCount;

			if ((TotalTime() - ElapsedTime) < 1.0)
			{
				return;
			}

			FPS = FrameCount;
			Miliseconds = 1000.0 / FPS;


			FrameCount = 0;
			ElapsedTime += 1.0;

		}

		inline void SetFrameLimit(f64 Limit)
		{
			FrameLimit = Limit;
			//FrameLimit = std::clamp(Limit, 24,)
		}

		void Reset()
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();

			m_StartTime = currentTime;
			m_LastUpdate = currentTime;
			m_StopTime = {};
			bIsStopped = false;
		}

		f64		DeltaTime = 0.0;
		int32	FrameCount = 0;
		int32	FPS = 0;
		f64		ElapsedTime = 0.0f;
		f64		Miliseconds = 0.0f;

		// TODO:
		// Set to limit max fps.
		f64		FrameLimit = 60.0;

	private:

		std::chrono::high_resolution_clock::time_point m_StartTime{};
		std::chrono::high_resolution_clock::time_point m_PausedTime{};
		std::chrono::high_resolution_clock::time_point m_StopTime{};
		std::chrono::high_resolution_clock::time_point m_LastUpdate{};
		std::chrono::high_resolution_clock::time_point m_CurrentTime{};

		bool bIsStopped = false;

	};
} // namespace Luden::Core

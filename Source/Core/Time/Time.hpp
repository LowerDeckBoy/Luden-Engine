#pragma once

#include <chrono>

namespace Luden::Time
{
	using Timestamp = std::chrono::high_resolution_clock::time_point;

	// Get current timepoint.
	inline Timestamp GetTimestamp()
	{
		return std::chrono::high_resolution_clock::now();
	}

	// Get duration in seconds based on given previous timepoint.
	inline double GetDuration(const Timestamp& PreviousTimestamp)
	{
		return std::chrono::duration<double>(GetTimestamp() - PreviousTimestamp).count();
	}

	// Get duration in miliseconds based on given previous timepoint.
	inline double GetDurationInMiliseconds(const Timestamp& PreviousTimestamp)
	{
		return std::chrono::duration<double, std::milli>(GetTimestamp() - PreviousTimestamp).count();
	}

	// Get duration in nanoseconds based on given previous timepoint.
	inline double GetDurationInNanoseconds(const Timestamp& PreviousTimestamp)
	{
		return std::chrono::duration<double, std::nano>(GetTimestamp() - PreviousTimestamp).count();
	}

} // namespace Luden::Time

#pragma once

#include <Psapi.h>

namespace Luden::Platform
{
	// Return how many RAM is being used by application.
	// Reads PrivateWorkingSetSize to match memory usage in Windows' Task Manager.
	static float GetMemoryUsage()
	{
		::PROCESS_MEMORY_COUNTERS_EX2 pcmex{};
		if (!::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pcmex, sizeof(::PROCESS_MEMORY_COUNTERS_EX2)))
		{
			return -1.0f;
		}

		return static_cast<float>((pcmex.PrivateWorkingSetSize) / (1024.0f * 1024.0f));
	}
} // Luden::Platform

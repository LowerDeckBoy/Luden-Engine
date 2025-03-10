#pragma once

#if defined PLATFORM_EXPORTS
	#define PLATFORM_API __declspec(dllexport)
#else
	#define PLATFORM_API __declspec(dllimport)
#endif

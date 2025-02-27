#pragma once

#if defined CORE_EXPORTS
	#define CORE_API __declspec(dllexport)
#else
	#define CORE_API __declspec(dllimport)
#endif

//#define CORE_API

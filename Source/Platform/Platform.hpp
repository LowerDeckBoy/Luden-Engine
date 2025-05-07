#pragma once

#if defined (_WIN64)
	#define PLATFORM_WIN64 1
#else
	#define PLATFORM_WIN64 0
#endif 

#if PLATFORM_WIN64

	#ifndef NOCOMM
	#define NOCOMM
	#endif
	#ifndef NOMINMAX
	#define NOMINMAX
	#endif
	#ifndef NOCRYPT
	#define NOCRYPT
	#endif
	#define NOTAPE
	#define NOIMAGE
	#define NOPROXYSTUB
	#define NORPC
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif

	#include <Windows.h>

	#if WINVER < _WIN32_WINNT_WIN10
		#error Windows version below 10 is not supported!
	#endif

#else
	#error OS other than Windows 10/11 is not supported!
#endif

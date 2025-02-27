#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "Logger.hpp"

BOOL WINAPI DllMain(
	HINSTANCE  hInstanceDLL ,
	DWORD CallReason,
	LPVOID /* lpvReserved */
)
{
	switch (CallReason)
	{
	case DLL_PROCESS_ATTACH:
		LOG_DEBUG("DLL_PROCESS_ATTACH");
		break;
	case DLL_PROCESS_DETACH:
		LOG_DEBUG("DLL_PROCESS_DETACH");
		break;
	case DLL_THREAD_ATTACH:
		LOG_DEBUG("DLL_THREAD_ATTACH");
		break;
	case DLL_THREAD_DETACH:
		LOG_DEBUG("DLL_THREAD_DETACH");
		break;
	}

	return TRUE;
}

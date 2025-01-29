#pragma once

#ifndef __cplusplus
	#error C++ is required to build this library!
#endif

#if defined (_DEBUG) || (DEBUG)
	#define CORE_DEBUG
#endif

#define CORE_STRINGIFY(Arg) #Arg
#define CORE_STRING(Arg)	CORE_STRINGIFY(Arg)

#define CORE_VERSION_MAJOR 0
#define CORE_VERSION_MINOR 1
#define CORE_VERSION_PATCH 0

#define CORE_VERSION \
	CORE_STRING(CORE_VERSION_MAJOR.CORE_VERSION_MINOR.CORE_VERSION_PATCH)

#define INLINE		inline
#define FORCEINLINE __forceinline

#define DEPRECATED	[[deprecated]]
#define NODISCARD	[[nodiscard]]
#define FALLTHROUGH [[fallthrough]]

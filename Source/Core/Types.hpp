#pragma once

#include <cstdint>
#include <type_traits>

using int8		= std::int8_t;
using int16		= std::int16_t;
using int32		= std::int32_t;
using int64		= std::int64_t;

using uint8		= std::uint8_t;
using uint16	= std::uint16_t;
using uint32	= std::uint32_t;
using uint64	= std::uint64_t;

using f32		= float;
using f64		= double;

using usize		= std::size_t;

template<typename T>
constexpr bool IsFloatingPoint()
{
	return std::is_floating_point<T>();
}

/* =========================== Running x64 platform only =========================== */

static_assert(sizeof(int8)   == 1,	"Failed to assert *int8* size!" );
static_assert(sizeof(int16)  == 2,	"Failed to assert *int16* size!");
static_assert(sizeof(int32)  == 4,	"Failed to assert *int32* size!");
static_assert(sizeof(int64)  == 8 ,	"Failed to assert *int64* size!");

static_assert(sizeof(uint8)  == 1,  "Failed to assert *uint8* size!" );
static_assert(sizeof(uint16) == 2,	"Failed to assert *uint16* size!");
static_assert(sizeof(uint32) == 4,	"Failed to assert *uint32* size!");
static_assert(sizeof(uint64) == 8,	"Failed to assert *uint64* size!");

#pragma once

#include "../Export.hpp"
#include "../Types.hpp"
#include <cmath>

namespace Luden::Math
{
	constexpr f32 PI		= 3.1415926535f;
	constexpr f32 TwoPI		= 2.0f * PI;
	constexpr f32 HalfPi	= PI / 2.0f;
	constexpr f32 Epsilon	= 0.000001f;
	constexpr f32 Rad2Deg	= 57.29578f;
	constexpr f32 Deg2Rad	= 0.01745329251f;

	#define ALIGN(Value, Alignment) (Value + (Alignment - 1)) & ~(Alignment - 1)

	constexpr uint32 Clamp(uint32 Value, uint32 Min, uint32 Max)
	{
		return (Value < Min) ? Min : (Value > Max) ? Max : Value;
	}

	constexpr f32 Clamp(f32 Value, f32 Min, f32 Max)
	{
		return (Value < Min) ? Min : (Value > Max) ? Max : Value;
	}
	
} // namespace Luden::Math

#pragma once

#include "../Export.hpp"
#include "../Types.hpp"
#include <cmath>

namespace Luden::Math
{
	constexpr f32 PI		= 3.1415926535f;
	constexpr f32 TwoPI		= 2.0f * PI;
	constexpr f32 HalfPI	= PI / 2.0f;
	constexpr f32 Epsilon	= 0.000001f;
	constexpr f32 Rad2Deg	= 57.29578f;
	constexpr f32 Deg2Rad	= 0.01745329251f; // PI / 180.0f

	#define ALIGN(Value, Alignment) (Value + (Alignment - 1)) & ~(Alignment - 1)

	#define ROUND_UP(Value)	static_cast<uint32>((Value) + 1)

	template<typename T>
	constexpr T Align(T Value, T Alignment)
	{
		return (Value + (Alignment - 1)) & ~(Alignment - 1);
	}

	constexpr f32 ConvertToRadians(f32 Value)
	{
		return Value * Deg2Rad;
	}

	constexpr f32 ConvertToDegrees(f32 Value)
	{
		return Value * Rad2Deg;
	}

	constexpr uint32 Clamp(uint32 Value, uint32 Min, uint32 Max)
	{
		return (Value < Min) ? Min : (Value > Max) ? Max : Value;
	}

	template<typename T>
	constexpr T Clamp(T Value, T Min, T Max)
	{
		return (Value < Min) ? Min : (Value > Max) ? Max : Value;
	}

	constexpr f32 Clamp(f32 Value, f32 Min, f32 Max)
	{
		return (Value < Min) ? Min : (Value > Max) ? Max : Value;
	}

	template<typename T>
	constexpr T RoundUp(T Value)
	{
		return static_cast<T>(Value + 0.5f);
	}
} // namespace Luden::Math

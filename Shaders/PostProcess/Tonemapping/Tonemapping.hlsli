#ifndef TONEMAPPING_HLSL
#define TONEMAPPING_HLSL

// https://github.com/NVIDIA-RTX/RTXGI/blob/main/Samples/Pathtracer/Tonemapping.hlsl

#include "../../Common/Bindless.hlsli"
#include "../../Common/Color.hlsli"

namespace Tonemapping
{
	static const uint TypeReinhard			= 1;
	static const uint TypeGammaCorrection	= 2;
	static const uint TypeUncharted2		= 3;
	static const uint TypeACESFilm			= 4;
	static const uint TypeHable				= 5;

	float3 TonemapReinhard(float3 Color)
	{
		const float luminance = GetLuminance(Color);
		const float reinhard = luminance / (1.0f + luminance);
		
		return saturate(Color * (reinhard / luminance));
	}
	
	float3 TonemapGammaCorrection(float3 Color)
	{
		float3 output = pow(Color, 0.4545454545f);
		
		return saturate(output);
	}
	
	float3 TonemapUncharted2(float3 Color)
	{
		const float A = 0.15f;
		const float B = 0.50f;
		const float C = 0.10f;
		const float D = 0.20f;
		const float E = 0.02f;
		const float F = 0.30f;

		return saturate(((Color * (A * Color + C * B) + D * E) / (Color * (A * Color + B) + D * F)) - E / F);
	}

	float3 TonemapACES(float3 Color)
	{
		Color  *= 0.6f;
		const float a = 2.51f;
		const float b = 0.03f;
		const float c = 2.43f;
		const float d = 0.59f;
		const float e = 0.14f;
		
		return saturate((Color * (a * Color + b)) / (Color * (c * Color + d) + e));
	}

	float3 TonemapHable(float3 Color)
	{
		const float A = 0.15f;
		const float B = 0.50f;
		const float C = 0.10f;
		const float D = 0.20f;
		const float E = 0.02f;
		const float F = 0.30f;
		const float W = 11.2f;
		const float exposure = 2.0f;

		Color *= exposure;
		Color = ((Color * (A * Color + C * B) + D * E) / (Color * (A * Color + B) + D * F)) - E / F;
		
		float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
		Color /= white;

		return saturate(Color);
	}
	
	float3 ApplyTonemapping(float3 Color, float ExposureScale, uint Type)
	{
		//const float3 output = Color * ExposureScale;
		const float3 output = Color;
		
		switch (Type)
		{
			case 0:	// No filter.
				return Color;
			case TypeReinhard:
				return TonemapReinhard(output * ExposureScale);
			case TypeGammaCorrection:
				return TonemapGammaCorrection(output);
			case TypeUncharted2:
				return TonemapUncharted2(output);
			case TypeACESFilm:
				return TonemapACES(output);
			case TypeHable:
				return TonemapHable(output);
			default:
				return Color;
		}
	}
	
} // namespace Tonemapping

#endif // TONEMAPPING_HLSL

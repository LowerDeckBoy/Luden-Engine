#ifndef TONEMAPPING_HLSL
#define TONEMAPPING_HLSL

// https://github.com/NVIDIA-RTX/RTXGI/blob/main/Samples/Pathtracer/Tonemapping.hlsl

#include "../../Common/Bindless.hlsli"
#include "../../Common/Color.hlsli"

namespace Tonemapping
{
	static const uint TypeACESFilm			= 1;
	static const uint TypeAgX				= 2;
	static const uint TypeAgXPunchy			= 3;
	static const uint TypeAgXGolden			= 4;
	static const uint TypeReinhard			= 5;
	static const uint TypeGammaCorrection	= 6;
	static const uint TypeUncharted2		= 7;
	static const uint TypeHable				= 8;

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
				
	static const float3x3 AgXTransformMatrix = float3x3(
		0.8424010709504686f,	0.04240107095046854f,	0.04240107095046854f,
		0.07843650156180276f,	0.8784365015618028f,	0.07843650156180276f,
		0.0791624274877287f,	0.0791624274877287f,	0.8791624274877287f
	);
	
	static const float3x3 AgXTransformInvMatrix = float3x3(
		 1.1969986613119143f,	-0.053001338688085674f, -0.053001338688085674f,
		-0.09804562695225345f,	 1.1519543730477466f,	-0.09804562695225345f,
		-0.09895303435966087f,	-0.09895303435966087f,	 1.151046965640339f
	);
		
	float3 AgXContrastApproximation(float3 Color)
	{
		float3 x2 = Color * Color;
		float3 x4 = x2 * x2;
		
		return  + 15.5f * x4 * x2
				- 40.14f * x4 * Color
				+ 31.96f * x4
				- 6.868f * x2 * Color
				+ 0.4298f * x2
				+ 0.1191f * Color
				- 0.00232f;
	}
	
	float3 AgX(float3 Color)
	{
		Color = mul(Color, AgXTransformMatrix);
		
		const float minEv = -12.473931188332413f;
		const float maxEv = 4.026068811667588f;

		Color = clamp(log2(Color), minEv, maxEv);
		Color = (Color - minEv) / (maxEv - minEv);
		
		return AgXContrastApproximation(Color);
	}
	
	float3 AgxEotf(float3 Color)
	{
		Color = mul(Color, AgXTransformInvMatrix);
  
		// Currently using sRGB texture format.
		//Color = pow(Color, 2.2f);

		return Color;
	}
	
	float3 TonemapAgX(float3 Color)
	{
		Color = AgX(Color);
	
		float luma = GetLuminance(Color);
  
		float3	offset		= float3(0.0f, 0.0f, 0.0f);
		float3	slope		= float3(1.0f, 1.0f, 1.0f);
		float3	power		= float3(1.0f, 1.0f, 1.0f);
		float	saturation	= 1.0f;
  
		Color = pow(Color * slope + offset, power);
		Color = luma + saturation * (Color - luma);
		Color = AgxEotf(Color);

		return saturate(Color);
	}
	
	float3 TonemapAgXPunchy(float3 Color)
	{
		Color = AgX(Color);

		float luma = GetLuminance(Color);
  
		float3	offset		= float3(0.0f, 0.0f, 0.0f);
		float3	slope		= float3(1.0f, 1.0f, 1.0f);
		float3	power		= float3(1.35f, 1.35f, 1.35f);
		float	saturation	= 1.4f;
  
		Color = pow(Color * slope + offset, power);
		Color = luma + saturation * (Color - luma);
		Color = AgxEotf(Color);

		return saturate(Color);
	}
	
	float3 TonemapAgXGolden(float3 Color)
	{
		Color = AgX(Color);

		float luma = GetLuminance(Color);
  
		float3 offset		= float3(0.0f, 0.0f, 0.0f);
		float3 slope		= float3(1.0f, 0.9f, 0.5f);
		float3 power		= float3(0.8f, 0.8f, 0.8f);
		float  saturation	= 0.8f;
  
		Color = pow(Color * slope + offset, power);
		Color = luma + saturation * (Color - luma);
		Color = AgxEotf(Color);

		return saturate(Color);
	}

	float3 ApplyTonemapping(float3 Color, float ExposureScale, uint Type)
	{
		const float3 output = Color;
		
		switch (Type)
		{
			case 0:							return saturate(Color);
			case TypeACESFilm:				return TonemapACES(output);
			case TypeAgX:					return TonemapAgX(output);
			case TypeAgXPunchy:				return TonemapAgXPunchy(output);
			case TypeAgXGolden:				return TonemapAgXGolden(output);
			case TypeReinhard:				return TonemapReinhard(output * ExposureScale);
			case TypeGammaCorrection:		return TonemapGammaCorrection(output);
			case TypeUncharted2:			return TonemapUncharted2(output);
			case TypeHable:					return TonemapHable(output);
			default:						return saturate(Color);
		}
	}
	
} // namespace Tonemapping

#endif // TONEMAPPING_HLSL

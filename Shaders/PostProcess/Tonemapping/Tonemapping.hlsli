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
	static const uint TypeAGX				= 5;
	static const uint TypeHable				= 6;

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

	float3 AGXCurve3(float3 Color)
	{
		const float threshold 	= 0.6060606060606061f;
   		const float aUp 		= 69.86278913545539f;
   		const float aDown 		= 59.507875f;
   		const float bUp 		= 13.0f / 4.0f;
   		const float bDown 		= 3.0f / 1.0f;
   		const float cUp 		= -4.0f / 13.0f;
   		const float cDown 		= -1.0f / 3.0f;

    	float3 mask = step(Color, float3(threshold, threshold, threshold));
    	float3 a = aUp + (aDown - aUp) * mask;
    	float3 b = bUp + (bDown - bUp) * mask;
    	float3 c = cUp + (cDown - cUp) * mask;

    	return 0.5f + (((-2.0f * threshold)) + 2.0f * Color) * pow(1.0f + a * pow(abs(Color - threshold), b), c);
	}

	float3 TonemapAGX(float3 Color)
	{
		Color = pow(Color, 2.2);

 		const float minEv 			= -12.473931188332413f;
    	const float maxEv 			= 4.026068811667588f;
    	const float dynamicRange 	= maxEv - minEv;

    	const float3x3 agxMatrix = float3x3(
			0.8424010709504686f, 	0.04240107095046854f, 	0.04240107095046854f, 
			0.07843650156180276f, 	0.8784365015618028f, 	0.07843650156180276f, 
			0.0791624274877287f, 	0.0791624274877287f, 	0.8791624274877287f
			);

    	const float3x3 agxMatrixInv = float3x3(
			1.1969986613119143f, 	-0.053001338688085674f, 	-0.053001338688085674f,
			-0.09804562695225345f, 	1.1519543730477466f, 		-0.09804562695225345f, 
			-0.09895303435966087f, 	-0.09895303435966087f, 		1.151046965640339f
			);

    	Color = mul(Color, agxMatrix);

   		float3 ct = saturate(log2(Color) * (1.0 / dynamicRange) - (minEv / dynamicRange));
   		float3 output = AGXCurve3(ct);

    	output = mul(output, agxMatrixInv);

    	return output;
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
			case TypeAGX:
				return TonemapAGX(output);
			case TypeHable:
				return TonemapHable(output);
			default:
				return Color;
		}
	}
	
} // namespace Tonemapping

#endif // TONEMAPPING_HLSL

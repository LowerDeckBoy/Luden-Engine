#ifndef TONEMAPPING_HLSL
#define TONEMAPPING_HLSL

// https://github.com/NVIDIA-RTX/RTXGI/blob/main/Samples/Pathtracer/Tonemapping.hlsl

namespace Tonemapping
{
	static const uint TypeReinhard			= 1;
	static const uint TypeGammaCorrection	= 2;
	static const uint TypeUncharted2		= 3;
	static const uint TypeACESFilm			= 4;

	float GetLuminance(float3 Color)
	{
		return dot(Color, float3(0.299f, 0.587f, 0.114f));
	}

	float3 TonemapReinhard(float3 Color)
	{
		const float luminance = GetLuminance(Color);
		const float reinhard = luminance / (1.0f + luminance);
		
		return Color * (reinhard / luminance);
	}
	
	float3 TonemapGammaCorrection(float3 Color)
	{
		float3 output = pow(Color, 0.4545454545f);
		
		return output;
	}
	
	float3 TonemapUncharted2(float3 Color)
	{
		float A = 0.15f;
		float B = 0.50f;
		float C = 0.10f;
		float D = 0.20f;
		float E = 0.02f;
		float F = 0.30f;

		return ((Color * (A * Color + C * B) + D * E) / (Color * (A * Color + B) + D * F)) - E / F;
	}

	float3 TonemapACES(float3 Color)
	{
		float a = 2.51f;
		float b = 0.03f;
		float c = 2.43f;
		float d = 0.59f;
		float e = 0.14f;
		
		return saturate((Color * (a * Color + b)) / (Color * (c * Color + d) + e));
	}
	
	float3 ApplyTonemapping(float3 Color, float ExposureScale, uint Type)
	{
		const float3 output = Color * ExposureScale;
		
		switch (Type)
		{
			case 0:	// No filter.
				return Color;
			case TypeReinhard:
				return TonemapReinhard(output);
			case TypeGammaCorrection:
				return TonemapGammaCorrection(output);
			case TypeUncharted2:
				return TonemapUncharted2(output);
			case TypeACESFilm:
				return TonemapACES(output);
		}
	
		return Color;
	}
	
} // namespace Tonemapping

#endif // TONEMAPPING_HLSL

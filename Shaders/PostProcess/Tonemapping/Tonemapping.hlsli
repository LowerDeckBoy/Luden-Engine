#ifndef TONEMAPPING_HLSL
#define TONEMAPPING_HLSL

// https://github.com/NVIDIA-RTX/RTXGI/blob/main/Samples/Pathtracer/Tonemapping.hlsl

namespace Tonemapping
{
	static const uint TypeReinhard			= 1;
	static const uint TypeGammaCorrection	= 2;
	static const uint TypeUncharted2		= 3;

	float GetLuminance(float3 Color)
	{
		return dot(Color, float3(0.299f, 0.587f, 0.114f));
	}

	float3 GetReinhard(float3 Color)
	{
		const float luminance = GetLuminance(Color);
		const float reinhard = luminance / (luminance + 1.0f);
		
		return Color * (reinhard / luminance);
	}
	
	float3 GammaCorrection(float3 Color)
	{
		float3 output = pow(Color, 0.4545454545f);
		
		return output;
	}
	
	float3 Uncharted2Tonemapping(float3 Color)
	{
		float A = 0.15;
		float B = 0.50;
		float C = 0.10;
		float D = 0.20;
		float E = 0.02;
		float F = 0.30;

		return ((Color * (A * Color + C * B) + D * E) / (Color * (A * Color + B) + D * F)) - E / F;
	}

	float3 Tonemap(float3 Color, float ExposureScale, uint Type)
	{
		const float3 output = Color * ExposureScale;
		
		switch (Type)
		{
			case 0:
				return Color;
			case TypeReinhard:
				return GetReinhard(output);
			case TypeGammaCorrection:
				return GammaCorrection(output);
			case TypeUncharted2:
				return Uncharted2Tonemapping(output);
		}
	
		return Color;
		//return GetReinhard(output);
	}
	
} // namespace Tonemapping

#endif // TONEMAPPING_HLSL

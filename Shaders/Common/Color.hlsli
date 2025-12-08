#ifndef COLOR_HLSLI
#define COLOR_HLSLI

const static float Gamma = 2.2f;

float GetLuminance(float3 Color)
{
	return max(dot(Color, float3(0.2126729f, 0.7151522f, 0.0721750f)), 0.0001f);
}

float3 LinearToSRGB(float3 Color)
{
	return pow(Color, 1.0f / Gamma);
}

float3 GetSubsurfaceScattering(float NdotL, float Radius)
{
	return exp(-3.0f * abs(NdotL) / (Radius + 0.001f));
}

float3 RGB2SRGB(float3 Color)
{
	float3 output;
	output.x = dot(float3( 3.2404542, -1.5371385, -0.4985314), Color);
	output.y = dot(float3(-0.9692660,  1.8760108,  0.0415560), Color);
	output.z = dot(float3( 0.0556434, -0.2040259,  1.0572252), Color);

	return output;
}

float3 SRGB2RGB(float3 Color)
{
	float3 output;
	output.x = dot(float3(0.4124564, 0.3575761, 0.1804375), Color);
	output.y = dot(float3(0.2126729, 0.7151522, 0.0721750), Color);
	output.z = dot(float3(0.0193339, 0.1191920, 0.9503041), Color);

	return output;
}

static const float3x3 REC2020ToLinearSRGB = float3x3(
		1.6605, -0.1246, -0.0182,
		-0.5876, 1.1329, -0.1006,
		-0.0728, -0.0083, 1.1187
	);

static const float3x3 LinearSRGBToREC2020 = float3x3(
		0.6274, 0.0691, 0.0164,
		0.3293, 0.9195, 0.0880,
		0.0433, 0.0113, 0.8956
	);


#endif // COLOR_HLSLI

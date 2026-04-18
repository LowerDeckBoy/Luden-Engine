#ifndef TRANSMITTANCE_HLSL
#define TRANSMITTANCE_HLSL

#include "Sky_RootSig.hlsli"
#include "SkyCommon.hlsli"

#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

#define STEP_COUNT 128
//#define STEP_COUNT 40

struct TransmittanceConstants
{
	uint TransmittanceIndex;
};

ConstantBuffer<TransmittanceConstants> Constants : register(b0);

float3 getSigmaT(float h)
{
	float3	rayleigh	= RayleighScattering * exp(-h / RayleighHeight);
	float	mie			= (MieScattering + MieAbsorption) * exp(-h / MieHeight);
	float3  ozone		= OzoneAbsorption * max(0.0f, 1.0f - 0.5f * abs(h - OzoneCenter) / OzoneThickness);

	return rayleigh + mie + ozone;
}

// https://github.com/AirGuanZ/AtmosphereRenderer/blob/main/asset/transmittance.hlsl
// https://github.com/elliahu/atmosphere/blob/main/shaders/transmittance.slang
[RootSignature(SKY_ROOT)]
[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> Transmittance = ResourceDescriptorHeap[Constants.TransmittanceIndex];
	
	uint width, height;
	Transmittance.GetDimensions(width, height);
	
	if (DispatchThreadID.x >= width || DispatchThreadID.y >= height)
	{
		return;
	}
	
	float2 texCoord = float2(DispatchThreadID.xy) + float2(0.5f, 0.5f);
	
	float theta = asin(lerp(-1.0f, 1.0f, texCoord.y / float(height)));
	float h = lerp(0.0f, AtmosphereRadius - PlanetRadius, texCoord.x / float(width));

	float2 origin = float2(0, PlanetRadius + h);
	float2 direction = float2(cos(theta), sin(theta));
	
	float t = 0.0f;
	if (!findClosestIntersectionWithCircle(origin, direction, PlanetRadius, t))
	{
		bool temp = findClosestIntersectionWithCircle(origin, direction, AtmosphereRadius, t);
	}
	
	float2 end = origin + t * direction;

	float3 sum = 0.0f;
	for (int i = 0; i < STEP_COUNT; ++i)
	{
		float2 pi = lerp(origin, end, float(i) / float(STEP_COUNT));
		float hi = length(pi) - PlanetRadius;
		float3 sigma = getSigmaT(hi);
		sum += sigma;
	}
	
	float3 result = exp(-sum * (t / float(STEP_COUNT)));
	
	Transmittance[DispatchThreadID.xy] = float4(result, 1.0f);
}

#endif // TRANSMITTANCE_HLSL

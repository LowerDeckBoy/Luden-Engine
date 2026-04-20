#ifndef TRANSMITTANCE_HLSL
#define TRANSMITTANCE_HLSL

#include "Sky_RootSig.hlsli"
#include "SkyCommon.hlsli"

#define THREAD_GROUP_SIZE_X 8
#define THREAD_GROUP_SIZE_Y 4

//#define STEP_COUNT 128
#define STEP_COUNT 400
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

	return rayleigh + mie.xxx + ozone;
}

// https://github.com/AirGuanZ/AtmosphereRenderer/blob/main/asset/transmittance.hlsl
// https://github.com/elliahu/atmosphere/blob/main/shaders/transmittance.slang
// https://publications.lib.chalmers.se/records/fulltext/203057/203057.pdf
// https://www.shadertoy.com/view/slSXRW
// https://godotshaders.com/shader/sky-sorta/
[RootSignature(SKY_ROOT)]
[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void CSMain(int3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> Transmittance = ResourceDescriptorHeap[Constants.TransmittanceIndex];
	
	uint width, height;
	Transmittance.GetDimensions(width, height);
	
	if (DispatchThreadID.x >= width || DispatchThreadID.y >= height)
	{
		return;
	}

	float2 uv = float2(DispatchThreadID.xy) / float2(width, height);
	
	float theta = asin(lerp(-1.0f, 1.0f, uv.y));
	float h = lerp(0.0f, AtmosphereRadius - PlanetRadius, uv.x);
	//float theta = asin(lerp(-1.0f, 1.0f, float(DispatchThreadID.y + 0.5f) / float(height)));
	//float h = lerp(0.0f, AtmosphereRadius - PlanetRadius, float(DispatchThreadID.x + 0.5f) / float(width));

	//float2 origin = float2(0, PlanetRadius + h); // AtmosphereRadius - h
	//float2 direction = float2(cos(theta), sin(theta));
	float3 origin	= float3(0, PlanetRadius + h, 0.0f); // AtmosphereRadius - h
	float3 direction = float3(cos(theta), sin(theta), 0.0f);
	
	float2 tt = 0.0f = GetSphereIntersection(origin, direction, PlanetCenter, PlanetRadius);
	//float t = 0.0f;
	//if (!findClosestIntersectionWithCircle(origin, direction, PlanetRadius, t))
	//{
	//	bool temp = findClosestIntersectionWithCircle(origin, direction, AtmosphereRadius, t);
	//}
	
	//float2 end = origin + t * direction;
	float3 end = origin + tt * direction;

	float3 sum = 0.0f.xxx;
	for (int i = 0; i < STEP_COUNT; ++i)
	{
		float2 pi = lerp(origin, end, float(i) / float(STEP_COUNT));
		float  hi = length(pi) - PlanetRadius;
		float3 sigma = getSigmaT(hi);
		sum += sigma;
	}
	
	//float3 result = exp(-sum * (t / float(STEP_COUNT)));
	float3 result = exp(-sum * float3(tt / float(STEP_COUNT), 0.0f));
	
	Transmittance[DispatchThreadID.xy] = float4(result, 1.0f);
}

#endif // TRANSMITTANCE_HLSL

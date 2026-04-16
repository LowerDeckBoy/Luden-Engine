#ifndef TRANSMITTANCE_HLSL
#define TRANSMITTANCE_HLSL

#include "Sky_RootSig.hlsli"
#include "SkyCommon.hlsli"

#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

//#define STEP_COUNT 1000
#define STEP_COUNT 8

struct TransmittanceConstants
{
	uint TransmittanceIndex;
};

//ConstantBuffer<SkyParameters> Parameters : register(b0);
ConstantBuffer<TransmittanceConstants> Constants : register(b0);

float3 getSigmaS(float h)
{
	float3 rayleigh = RayleighScattering * exp(-h / RayleighHeight);
	float mie = 3.996 * exp(-h / MieHeight);
	return rayleigh + mie;
}

float3 getSigmaT(float h)
{
	float3 rayleigh = RayleighScattering * exp(-h / RayleighHeight);
	float mie = (3.996 + 4.4) * exp(-h / MieHeight);
	//float mie = (3.996) * exp(-h / MieHeight);
	float3 ozone = (Nitrogen * 0.06f) * exp(-h / 8000);
	//float3 ozone = Nitrogen * max(0.0f, 1 - 0.5 * abs(h - OzoneCenter) / OzoneThickness); 
	
	//float3 rayleigh = RayleighScattering * exp(-h / RayleighHeight);
	//float3 rayleigh = RayleighScattering * GetRayleighHeightFactor(h);
	//float mie = (3.996 + 4.4) * GetMieHeightFactor(h);
	//float3 ozone = Nitrogen * GetOzoneHeightFactor(h);
	//float3 ozone = Nitrogen * max(0.0f, 1 - abs(h - OzoneCenter) / OzoneThickness);
	
	return rayleigh + mie.xxx + ozone;
}

void getSigmaST(float h, out float3 sigmaS, out float3 sigmaT)
{
	float3 rayleigh = RayleighScattering * exp(-h / RayleighHeight);

	float mieDensity = exp(-h / MieHeight);
	float mieS = 3.996 * mieDensity;
	float mieT = (3.996 + 4.4) * mieDensity;

	float3 ozone = float3(0.65f, 1.881f, 0.085f) * max(
        0.0f, 1 - 0.5 * abs(h - 25) / 30);

	sigmaS = rayleigh + mieS;
	sigmaT = rayleigh + mieT + ozone;
}

// https://github.com/AirGuanZ/AtmosphereRenderer/blob/main/asset/transmittance.hlsl
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
	
	float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f);
	
	float theta = asin(lerp(-1.0, 1.0, texCoord.y / height));
	float h = lerp(0.0f, AtmosphereRadius - EarthRadius, texCoord.x / width);

	float2 o = float2(0, EarthRadius + h);
	float2 d = float2(cos(theta), sin(theta));
	

	float t = 0;
	if (!findClosestIntersectionWithCircle(o, d, AtmosphereRadius, t))
	{
		bool temp = findClosestIntersectionWithCircle(o, d, EarthRadius, t);
	}
	
	float2 end = o + t * d;

	float3 sum;
	for (int i = 0; i < STEP_COUNT; ++i)
	{
		//float2 pi = lerp(o, end, (i + 0.5f) / STEP_COUNT);
		float2 pi = o + (i + 0.5f) * STEP_COUNT;
		float hi = length(pi) + EarthRadius;
		float3 sigma = getSigmaT(hi);
		sum += sigma;
	}
	
	float3 result = exp(-sum * (t / STEP_COUNT));
	
	Transmittance[DispatchThreadID.xy] = float4(result, 1.0f);
}

#endif // TRANSMITTANCE_HLSL

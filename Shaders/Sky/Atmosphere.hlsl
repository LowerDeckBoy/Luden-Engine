#ifndef ATMOSPHERE_HLSL
#define ATMOSPHERE_HLSL

#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"
#include "Atmosphere_RS.hlsli"

// https://cgg.mff.cuni.cz/projects/SkylightModelling/
// https://cgg.mff.cuni.cz/publications/skymodel-2021/

struct Parameters
{
	uint SceneImageIndex;
	uint OutputImageIndex;
	uint WorldPositionIndex;
	uint DepthIndex;

	float4 CameraPosition; // TODO: repack it later
	float4 SunPosition; // TODO: repack it later

	float Brightness; // Intensity

	float RadiusAtmosphere; // Outer Radius
	float RadiusPlanetery; // Inner Radius
	float padding;
	float3 ScatteringCoefficiencyRayleigh; // Kr
	float ScatteringCoefficiencyMie; // Km
	float ScaleHeightRayleigh; // Hr
	float ScaleHeightMie; // Hm
	float ScatteringDirection; // Gm
};

#define DISPATCH_BLOCK 16

const static float Hr = 8000;
const static float Hm = 1200;
const static float Ho = 8000;

const static float3 rayleigh = float3(5.8, 13.5, 33.1) * 1e-6;
const static float3 mie = float3(21, 21, 21) * 1e-6;
const static float3 ozone = float3(3.426, 8.298, 0.356) * 0.06 * 1e-5;
const static float radiusEarth = 6360e3;
const static float radiusAtmo = 6420e3;
const static float ZenithH = radiusAtmo - radiusEarth;
const static float3 origin_view = float3(0, radiusEarth + 1, 0);
const static float originH = origin_view.y - radiusEarth;
const static int STEPS = 8;

ConstantBuffer<Parameters> Constants : register(b0);

float3 GetRayIntersection(float3 RayOrigin, float3 RayDirection, float SphereRadius)
{
	// f(x) = a(x^2) + bx + c
	float b = dot(RayOrigin, RayDirection);
	float c = dot(RayOrigin, RayOrigin) - SphereRadius * SphereRadius;
	float discriminant = b * b - c;
	float output = -b + sqrt(discriminant);
	return output;
}

float GetRayleighPhase(float3 CameraDirection, float3 SunDirection)
{
	const float L = dot(CameraDirection, SunDirection);
	return (3.0f / (16.0f * PI)) * (1.0f + L * L);
}

float GetMiePhase(float3 CameraDirection, float3 SunDirection) 
{
	float mu = dot(CameraDirection, SunDirection);
	float g = 0.76f;
	float denom = 1.0f + g * g - 2.0f * g * mu;
	return (1.0f - g * g) / (4.0f * PI * pow(denom, 1.5f));
}


// https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering
// https://github.com/EldarMuradov/EraEngine/blob/master/modules/shaders/atmosphere_cs.hlsl

[RootSignature(ATMOSPHERE_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID: SV_DispatchThreadID)
{
	Texture2D<float4> worldPositionTexture = GetTexture(Constants.WorldPositionIndex);
	RWTexture2D<float4> outputTexture = GetRWTexture<float4>(Constants.OutputImageIndex);

	float3 worldPosition = worldPositionTexture[DispatchThreadID.xy].xyz;
	
	float3 V = Constants.CameraPosition.xyz - worldPosition;

	float far = length(V);
	V /= far;
	
	const uint numInScatteringPoints = 30;

	const float3 minCorner = float3(-80.f, 0.f, -80.f);
	const float3 maxCorner = float3(80.f, 50.f, 80.f);
	
	//outputTexture[DispatchThreadID.xy] = float4(V, 1.0f);
	float mie = GetMiePhase(Constants.CameraPosition.xyz, Constants.SunPosition.xyz);
	//outputTexture[DispatchThreadID.xy] = float4(mie, mie, mie, 1.0f);
	outputTexture[DispatchThreadID.xy] = float4(V, 1.0f);
}

#endif // ATMOSPHERE_HLSL

#ifndef ATMOSPHERE_HLSL
#define ATMOSPHERE_HLSL

#include "Atmosphere_RS.hlsli"

struct Parameters
{

};

const static uint DispatchBlock = 16;

const float PI = 3.14159265359;
const float Hr = 8000;
const float Hm = 1200;
const float Ho = 8000;

const float3 rayleigh = float3(5.8, 13.5, 33.1) * 1e-6;
const float3 mie = float3(21, 21, 21) * 1e-6;
const float3 ozone = float3(3.426, 8.298, 0.356) * 0.06 * 1e-5;
const float radiusEarth = 6360e3;
const float radiusAtmo = 6420e3;
const float ZenithH = radiusAtmo - radiusEarth;
const float3 origin_view = float3(0, radiusEarth + 1, 0);
const float originH = origin_view.y - radiusEarth;
const int STEPS = 8;

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

float GetMiePhase(float3 CameraDirection, float3 SunDirection) {
    float mu = dot(CameraDirection, SunDirection);
    float g = 0.76f;
    float denom = 1.0f + g * g - 2.0f * g * mu;
    return (1.0f - g * g) / (4.0f * PI * pow(denom, 1.5f));
}

[numthreads(DispatchBlock, DispatchBlock, 1)]
void CSMain(uint3 DispatchThreadID: SV_DispatchThreadID)
{


}

#endif // ATMOSPHERE_HLSL

#ifndef SKY_COMMON_HLSLI
#define SKY_COMMON_HLSLI

#include "../Common/Common.hlsli"

static const float3 RayleighScattering = float3(5.802f, 13.558f, 33.100f);//* 1e-6f;
static const float3 MieScattering = float3(3.996f, 3.996f, 3.996f);//* 1e-6f;
static const float3 OzoneScattering = float3(0.0f, 0.0f, 0.0f);

static const float3 RayleighAbsorption = float3(0.0f, 0.0f, 0.0f);
static const float3 MieAbsorption = float3(4.40, 4.40, 4.40);//* 1e-6f;
static const float3 OzoneAbsorption = float3(0.650f, 1.881f, 0.085f);//* 1e-6f;

static const float3 Nitrogen = float3(0.650f, 1.881f, 0.085f);//* 1e-6f;
static const float3 NitrogenExtinction = float3(0.000650f, 0.001881f, 0.000085f);

static const float3 GroundFactor = float3(0.3f, 0.3f, 0.3f);

static const float IsotropicPhaseFactor = 1.0f / (4.0f * PI);

static const float AtmosphereHeight = 6371.0f;
static const float RayleighHeight = 8.0f;
static const float MieHeight = 1.2f;
static const float MieG = 0.8f;
static const float OzoneCenter = 25.0f;
static const float OzoneThickness = 30.0f;

static const float3 UpVector = float3(0.0f, 1.0f, 0.0f);

static const float EarthRadius = 6360.0f; // 6360km
static const float AtmosphereRadius = 6420.0f; // 6420km
static const float3 EarthCenter = float3(0, -EarthRadius, 0); // Or 0, 0, 0 for now

static const float AirIOR = 1.0003f;

// cos theta
float GetRayleighPhase(float VdotL)
{
	return (3.0f * (1.0f + VdotL * VdotL)) / (16.0f * PI);
}

float GetMiePhase(float VdotL, float G = 0.85f)
{
	const float g2 = MieG * MieG;
	
	return (3.0f / (8.0f * PI)) * ((((1.0f - g2) * (1.0f + VdotL * VdotL)) / pow(((2.0f + g2) * (1.0f + g2 - (2.0f * MieG * VdotL))), 1.5f)));
}

float GetRayleighHeightFactor(float Height)
{
	return exp(-Height * (1.0f / RayleighHeight));
}

float GetMieHeightFactor(float Height)
{
	return exp(-Height * (1.0f / MieHeight));
}

float GetOzoneHeightFactor(float Height)
{
	return max(0.0f, 1.0f - abs(Height - 25.0f) / 15.0f);
}

float2 GetSphereIntersection(float3 Origin, float3 Direction, float3 SphereCenter, float SphereRadius)
{
	Origin -= SphereCenter;
	float a = dot(Direction, Direction);
	float b = 2.0 * dot(Origin, Direction);
	float c = dot(Origin, Origin) - (SphereRadius * SphereRadius);
	float d = b * b - 4 * a * c;
	
	if (d < 0)
	{
		return -1;
	}

	d = sqrt(d);
	return float2(-b - d, -b + d) / (2 * a);
}

// 
bool hasIntersectionWithCiecle(float2 o, float2 d, float R)
{
	float A = dot(d, d);
	float B = 2 * dot(o, d);
	float C = dot(o, o) - R * R;
	float delta = B * B - 4 * A * C;
	return (delta >= 0) && ((C <= 0) | (B <= 0));
}

bool hasIntersectionWithSphere(float3 o, float3 d, float R)
{
	float A = dot(d, d);
	float B = 2 * dot(o, d);
	float C = dot(o, o) - R * R;
	float delta = B * B - 4 * A * C;
	return (delta >= 0) && ((C <= 0) | (B <= 0));
}

bool findClosestIntersectionWithCircle(
    float2 o, float2 d, float R, out float t)
{
	float A = dot(d, d);
	float B = 2 * dot(o, d);
	float C = dot(o, o) - R * R;
	float delta = B * B - 4 * A * C;
	if (delta < 0)
	{
		t = 0.0f;
		return false;
	}
	t = (-B + (C <= 0 ? sqrt(delta) : -sqrt(delta))) / (2 * A);
	
	return (C <= 0) | (B <= 0);
}

bool findClosestIntersectionWithSphere(
    float3 o, float3 d, float R, out float t)
{
	float A = dot(d, d);
	float B = 2 * dot(o, d);
	float C = dot(o, o) - R * R;
	float delta = B * B - 4 * A * C;
	if (delta < 0)
	{
		t = 0.0f;
		return false;
	}
	t = (-B + (C <= 0 ? sqrt(delta) : -sqrt(delta))) / (2 * A);
	
	return (C <= 0) | (B <= 0);
}

#endif // SKY_COMMON_HLSLI

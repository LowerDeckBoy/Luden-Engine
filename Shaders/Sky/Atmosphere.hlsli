#ifndef ATMOSPHERE_HLSLI
#define ATMOSPHERE_HLSLI

// https://github.com/Gaukler/PlainRenderer/blob/master/resources/shaders/sky.inc

#define INFINITY 1.0 / 0.0

float3 GetSunColor(float3 Color, float3 V)
{
	return acos(normalize(dot(Color, V)));
}

float3 GetSphereIntersection(float3 RayStart, float3 RayDirection, float3 SphereCenter, float SphereRadius)
{
	RayStart -= SphereCenter;
	float a = dot(RayDirection, RayDirection);
	float b = 2.0f * dot(RayStart, RayDirection);
	float c = dot(RayStart, RayStart) - (SphereRadius * SphereRadius);
	float d = b * b - 4.0f * a * c;
	
	if (d < 0.0f)
	{
		return -1.0f;
	}

	d = sqrt(d);
	
	return float2(-b - d, -b + d) / (2.0f * a);
}

float RayleighHeightFactor(float Height)
{
	return exp(-Height * (1.0f / 8.0f));
}

float MieHeightFactor(float Height)
{
	return exp(-Height * (1.0f / 1.2f));
}

float OzoneHeightFactor(float Height)
{
	return max(0.0f, 1.0f - abs(Height - 25.0f) / 15.0f);
}

float3 RayleightPhase();


#endif // ATMOSPHERE_HLSLI
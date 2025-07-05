#ifndef LIGHT_HLSLI
#define LIGHT_HLSLI

struct PointLight
{
	float3	Position;
	float	Intensity;
	float3	Ambient;
	float	Radius;
};

#endif // LIGHT_HLSLI

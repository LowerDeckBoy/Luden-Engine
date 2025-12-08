#ifndef RAYTRACING_COMMON_HLSL
#define RAYTRACING_COMMON_HLSL

struct SceneData
{
	float4x4 View;
	float4x4 Projection;
	float4x4 InvViewProjection;
	
	float3 CameraPosition;
	uint padding;
	uint RaytracingOutput;
};

ConstantBuffer<SceneData> Scene	: register(b0, space0);
RaytracingAccelerationStructure gSceneBVH : register(t0, space0);

struct HitInfo
{
	float4 Color;
};

struct RayAttributes
{
	float2 Barycentrics;
};

float3 GetBarycentrics(in RayAttributes Attributes)
{
	return float3(
		1.0f - Attributes.Barycentrics.x - Attributes.Barycentrics.y,
		Attributes.Barycentrics.x,
		Attributes.Barycentrics.y);
}

float3 GetBarycentrics(in BuiltInTriangleIntersectionAttributes Attributes)
{
	return float3(
		1.0f - Attributes.barycentrics.x - Attributes.barycentrics.y,
		Attributes.barycentrics.x,
		Attributes.barycentrics.y);
}

float3 GetHitWorldPosition()
{
	return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

#endif // RAYTRACING_COMMON_HLSL

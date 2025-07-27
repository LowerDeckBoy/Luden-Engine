#ifndef RAYTRACING_COMMON_HLSL
#define RAYTRACING_COMMON_HLSL

struct SceneData
{
	float4x4 View;
	float4x4 Projection;
	float4x4 ViewProjection;
	
	float3 CameraPosition;
	uint padding;
	uint RaytracingOutput;
	uint RaytracingTopLevel;
};

ConstantBuffer<SceneData>		Scene			: register(b0, space0);

struct HitInfo
{
	float4 Color;
};

#endif // RAYTRACING_COMMON_HLSL

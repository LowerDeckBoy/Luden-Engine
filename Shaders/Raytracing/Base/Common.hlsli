#ifndef RAYTRACING_COMMON_HLSL
#define RAYTRACING_COMMON_HLSL



struct SceneData
{
	float4x4 View;
	float4x4 Projection;
	float4x4 ViewProjection;
	
	float3 CameraPosition;
	uint RaytracingOutput;
};

ConstantBuffer<SceneData> Scene : register(b0, space1);
//RWTexture2D<float4> gRaytraceScene : register(u0, space2);
RaytracingAccelerationStructure gSceneBVH : register(t0, space1);

struct HitInfo
{
	float4 Color;
};

#endif // RAYTRACING_COMMON_HLSL


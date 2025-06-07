#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

#include "Common.hlsli"

[shader("raygeneration")]
void RayGen()
{
	HitInfo payload = { float4(0.0f, 0.0f, 0.0f, 0.0f) };
	
	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dimensions = DispatchRaysDimensions().xy;
	
	float2 xy = launchIndex + 0.5f;
	float2 screenPos = xy / dimensions * 2.0 - 1.0;
	
	screenPos.y = -screenPos.y;
	float aspectRatio = dimensions.x / dimensions.y;
	float4x4 viewProj = Scene.ViewProjection;
	
	float4 world = mul(float4(screenPos, 0.0f, 1.0f), viewProj);
	world.xyz /= world.w;
	
	float3 origin = Scene.CameraPosition;
	float3 direction = normalize(world.xyz - origin);

	RayDesc ray;
	ray.Origin		= origin;
	ray.Direction	= direction;
	ray.TMin		= 0.01f;
	ray.TMax		= 10000.0f;

	
	TraceRay(gSceneBVH,
        RAY_FLAG_NONE,
        0xFF,
        0,
        1,
        0,
        ray,
        payload);
    
	RWTexture2D<float4> gRaytraceScene = ResourceDescriptorHeap[Scene.RaytracingOutput];
	gRaytraceScene[launchIndex] = payload.Color;

}

#endif // RAYGEN_HLSL

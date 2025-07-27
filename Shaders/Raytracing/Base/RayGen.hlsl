#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

#include "Common.hlsli"

[shader("raygeneration")]
void RayGen()
{
	HitInfo payload = { float4(0.0f, 0.0f, 0.0f, 0.0f) };
	
	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);
	float2 d = ((launchIndex.xy + 0.5f) / dimensions.xy) * 2.0f - 1.0f;
	
	float2 xy = launchIndex + 0.5f;
	float2 screenPos = xy / dimensions * 2.0f - 1.0f;
	
	screenPos.y = -screenPos.y;
	float aspectRatio = dimensions.x / dimensions.y;
	float4x4 viewProj = Scene.ViewProjection;
	
	float4 world = mul(float4(screenPos, 0.0f, 1.0f), viewProj);
	world.xyz /= world.w;
	
	float3 origin = Scene.CameraPosition;
	float3 direction = normalize(world.xyz - origin);

	RayDesc ray;
	//ray.Origin		= origin;
	//ray.Direction	= direction;
	ray.Origin		= float3(d.x, -d.y, 1.0f);
	ray.Direction	= float3(0.0f, 0.0f, -1.0f);
	ray.TMin		= 0.0f;
	ray.TMax		= 100000.0f;

	RWTexture2D<float4> gRaytraceScene = ResourceDescriptorHeap[Scene.RaytracingOutput];
	RaytracingAccelerationStructure gSceneBVH = ResourceDescriptorHeap[Scene.RaytracingTopLevel];
	
	TraceRay(gSceneBVH,
        RAY_FLAG_NONE,
        0xFF,
        0,
        0,
        0,
        ray,
        payload);
    
	gRaytraceScene[launchIndex] = float4(payload.Color.rgb, 1.0f);
	//float4 test = gRaytraceScene.Load(int3(launchIndex.xy, 0.0f));
	//test[launchIndex.xy] = payload.Color;

}

#endif // RAYGEN_HLSL

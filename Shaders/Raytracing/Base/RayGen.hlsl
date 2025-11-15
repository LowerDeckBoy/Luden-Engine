#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

#include "RaytraceCommon.hlsli"

RayDesc GenerateCameraRay()
{
	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);
	float2 uv = (float2(launchIndex.xy + 0.5f) / dimensions.xy) * 2.0f - 1.0f;
	
	uv.y = -uv.y;
	float aspectRatio = dimensions.x / dimensions.y;
	
	float4 world = mul(Scene.InvViewProjection, float4(uv, 0.0f, 1.0f));
	world.xyz /= world.w;
	
	float3 origin	 = Scene.CameraPosition;
	float3 direction = normalize(world.xyz - origin);

	RayDesc ray;
	ray.Origin		= origin;
	ray.Direction	= direction;
	ray.TMin		= 0.01f;
	ray.TMax		= 10000.0f;
	
	return ray;
}

[shader("raygeneration")]
void RayGen()
{
	HitInfo payload;
	payload.Color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	
	RayDesc ray = GenerateCameraRay();
	
	RWTexture2D<float4> gRaytraceScene = ResourceDescriptorHeap[Scene.RaytracingOutput];

	TraceRay(gSceneBVH,													// Scene Bounding Volume Hierarchy
		RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES,	// Ray flags
		0xFF,															// InstanceInclusionMask -> 0xFF to test all geometry
		0,																// RayContributionToHitGroupIndex
		1,																// MultiplierForGeometryContributionToHitGroupIndex
		0,																// MissShaderIndex
		ray,															// RayDesc
		payload															// User-defined payload
		);
	
	gRaytraceScene[DispatchRaysIndex().xy] = float4(payload.Color.rgb, 1.0f);

}

#endif // RAYGEN_HLSL

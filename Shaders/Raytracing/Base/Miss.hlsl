#ifndef MISS_HLSL
#define MISS_HLSL

#include "Common.hlsli"

[shader("miss")]
void Miss(inout HitInfo payload : SV_RayPayload)
{
	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dims = float2(DispatchRaysDimensions().xy);

	float ramp = launchIndex.y / dims.y;
	payload.Color = float4(0.5f, 0.2f, 0.7f - 0.3f * ramp, -1.0f);

}

#endif // MISS_HLSL

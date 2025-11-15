#ifndef CLOSEST_HIT_HLSL
#define CLOSEST_HIT_HLSL

#include "RaytraceCommon.hlsli"

[shader("closesthit")]
void ClosestHit(inout HitInfo Payload : SV_RayPayload, RayAttributes Attribs)
{
	float3 barycentrics = GetBarycentrics(Attribs);
	Payload.Color = float4(barycentrics, 1.0f);
}

#endif // CLOSEST_HIT_HLSL

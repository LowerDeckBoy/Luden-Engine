#ifndef CLOSESTHIT_HLSL
#define CLOSESTHIT_HLSL

#include "Common.hlsli"

[shader("closesthit")]
void ClosestHit(inout HitInfo Payload, BuiltInTriangleIntersectionAttributes Attribs)
{
	float3 color = float3(0.25f, 1.0f, 0.5f);
	Payload.Color = float4(color, RayTCurrent());

}

#endif // CLOSESTHIT_HLSL

#ifndef CLOSESTHIT_HLSL
#define CLOSESTHIT_HLSL

#include "Common.hlsli"

[shader("closesthit")]
void ClosestHit(inout HitInfo Payload, BuiltInTriangleIntersectionAttributes Attribs)
{
	//float3 color = float3(0.25f, 1.0f, 0.5f);
	float3 color = float3(1.0f, 1.0f, 1.0f);
	Payload.Color = float4(color, RayTCurrent());
	Payload.Color = float4(color, 1.0f);

}

#endif // CLOSESTHIT_HLSL

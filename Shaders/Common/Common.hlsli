#ifndef COMMON_HLSLI
#define COMMON_HLSLI

static const float PI		= 3.14159265358979323846f;
static const float TwoPI	= 6.28318530718;
static const float HalfPI	= 1.57079632679;
static const float InvPI	= 0.31830988618379067154f;

static const float Epsilon	= 0.0001f;

float2 TexelToUV(uint2 DispatchThreadID, float2 Texel)
{
	return (float2(DispatchThreadID.xy) + 0.5f) * Texel;
}

float4 GetWorldFromDepth(float2 UV, float Depth, float4x4 InvViewProjection)
{
	float2 ndc = UV * 2.0f - 1.0f;
	float4 clipPositionn = float4(ndc, Depth, 1.0f);
	
	float4 worldPosition = mul(InvViewProjection, clipPositionn);
	worldPosition /= worldPosition.w;

	return worldPosition;
}

float GetLinearDepth(float Depth, float Near, float Far)
{
	return Near * Far / (Far + (-Depth + 1.0f) * (Near - Far));
}

#endif // COMMON_HLSLI

#ifndef COMMON_HLSLI
#define COMMON_HLSLI

static const float PI		= 3.14159265358979323846f;
static const float TwoPI	= PI * 2.0f;
static const float HalfPI	= PI * 0.5f;
static const float InvPI	= 0.31830988618379067154f;

static const float3 Fdielectric = 0.16f; // 0.04f

static const float Epsilon	= 0.0001f;

static const float FLOAT_MIN = 1.175494351e-38f;
static const float FLOAT_MAX = 3.402823466e+38f;

static const uint  UINT32_MAX = 0xFFFFFFFF;

float DegreesToRadians(float Degrees)
{
	return Degrees * PI / 180.0f;
}

float2 TexelToUV(uint2 DispatchThreadID, float2 TexelSize)
{
	return (float2(DispatchThreadID.xy) + 0.5f) * TexelSize;
}

float GetLinearDepth(float Depth, float Near, float Far)
{
	return Near / (Near + Depth * (Far - Near));
}

float3 GetViewPosition(float2 UV, float Depth, float4x4 InversedProjection)
{
	float x = UV.x * 2.0f - 1.0f;
	float y = (1.0f - UV.y) * 2.0f - 1.0f;
	float z = Depth;
	
	float4 clipPosition = float4(x, y, z, 1.0f);

	float4 viewPosition = mul(clipPosition, InversedProjection);
	viewPosition.xyz /= viewPosition.w;

	return viewPosition.xyz;
}

float3 GetWorldPosition(float2 UV, float Depth, float4x4 InversedViewProjection)
{
	float x = UV.x * 2.0f - 1.0f;
	float y = (1.0f - UV.y) * 2.0f - 1.0f;
	float z = Depth;
	
	float4 clipPosition = float4(x, y, z, 1.0f);

	float4 worldPosition = mul(clipPosition, InversedViewProjection);
	worldPosition.xyz /= worldPosition.w;

	return worldPosition.xyz;
}

#endif // COMMON_HLSLI

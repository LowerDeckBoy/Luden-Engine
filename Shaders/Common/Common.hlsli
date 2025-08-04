#ifndef COMMON_HLSLI
#define COMMON_HLSLI

static const float PI		= 3.14159265358979323846f;
static const float TwoPI	= 6.28318530718;
static const float HalfPI	= 1.57079632679;
static const float InvPI	= 0.31830988618379067154f;

static const float Epsilon	= 0.0001f;

float2 GetTextureSize(in Texture2D Texture)
{
	float2 dimensions;
	Texture.GetDimensions(dimensions.x, dimensions.y);
	
	return dimensions;
}

float2 TexelToUV(uint2 DispatchThreadID, float2 Texel)
{
	return (float2(DispatchThreadID.xy) + 0.5f) * Texel;
}

float4 GetWorldFromDepth(float2 UV, float Depth, float4x4 InvViewProjection)
{
	float4 clipSpace = float4(UV * 2.0f - 1.0f, Depth, 1.0f);
	
	float4 worldPosition = mul(InvViewProjection, clipSpace);
	worldPosition /= worldPosition.w;

	return worldPosition;
}

float GetLinearDepth(float Depth, float Near, float Far)
{
	float depth = 2.0f * Depth - 1.0f;
	return (Near * Far) / (Far + depth * (Far - Near));
}

float3 Unproject(float2 UV, float Depth, row_major float4x4 InversedMatrix)
{
	float4 NDC = float4(UV * 2.0f - 1.0f, Depth, 1.0f);
	NDC.y *= -1.0f;
	//float4 world = mul(InversedMatrix, NDC);
	float4 world = mul(NDC, InversedMatrix);
	
	return world.xyz / world.w;
}

#endif // COMMON_HLSLI

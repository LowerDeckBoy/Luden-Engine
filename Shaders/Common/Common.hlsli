#ifndef COMMON_HLSLI
#define COMMON_HLSLI

static const float PI		= 3.14159265358979323846f;
static const float TwoPI	= 6.28318530718f;
static const float HalfPI	= 1.57079632679f;
static const float InvPI	= 0.31830988618379067154f;

static const float3 Fdielectric = 0.04f;

static const float Epsilon	= 0.0001f;

Texture2D GetBindlessTexture(uint Index)
{
	return ResourceDescriptorHeap[Index];
}

template<typename T>
RWTexture2D<T> GetBindlessRWTexture(uint Index)
{
	return ResourceDescriptorHeap[Index];
}

float2 GetTexelSize(float2 TextureSize)
{
	return 1.0f / TextureSize;
}

// Get texture X and Y dimensions.
float2 GetTextureSize(in Texture2D Texture)
{
	float2 dimensions;
	Texture.GetDimensions(dimensions.x, dimensions.y);
	
	return dimensions;
}

//
void GetTextureSize(in Texture2D Texture, out float2 TextureSize, out float2 TexelSize)
{
	TextureSize = GetTextureSize(Texture);
	TexelSize 	= GetTexelSize(TextureSize);
}

float2 GetRWTextureSize(in RWTexture2D<float4> RWTexture)
{
	float2 dimensions;
	RWTexture.GetDimensions(dimensions.x, dimensions.y);
	
	return dimensions;
}

template<typename T>
float2 GetTextureSize(in RWTexture2D<T> RWTexture)
{
	float2 dimensions;
	RWTexture.GetDimensions(dimensions.x, dimensions.y);
	
	return dimensions;
}

float DegreesToRadians(float Degrees)
{
	return Degrees * PI / 180.0f;
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

float4 GetWorldPositionFromDepth(float2 UV, float Depth, float4x4 InViewProjection)
{
	float4 H = float4(UV.x * 2.0f - 1.0f, (1.0f - UV.y) * 2.0f - 1.0f, Depth, 1.0f); // Transform by the view-projection inverse.
	float4 D = mul(H, InViewProjection); // Divide by w to get the world position.
	float4 worldPos = D / D.w;

	return worldPos;
}

float3 GetWorldPosition(float4 ClipPosition, float4x4 InvViewProjection)
{
	float4 world = mul(InvViewProjection, ClipPosition);
	world /= world.w;

	return world.xyz;
}

float GetLinearDepth(float Depth, float Near, float Far)
{
	float depth = 2.0f * Depth - 1.0f;
	return (Near * Far) / (Far + depth * (Far - Near));
}

float GetLinearizedDepth(float Depth, float Near, float Far)
{
	//return Far / (Far + Depth * (Near - Far));
	return Near * (Far / (Far + Depth * (Near - Far)));
}

float GetLinearDepthPerspective(float Depth, float Near, float Far)
{
	//float depth = 2.0f * Depth - 1.0f;
	float depth =  Depth ;
	//return (Near * Far) / (Far + depth * (Far - Near));
	return ((Far + Near) / (Far - Near)) + (1 / depth * (-2.0f * Far * Near) / (Far - Near));

}

float4 ClipToViewSpace(float4 ClipSpace, float4x4 InvProjection)
{
	float4 viewSpace = mul(ClipSpace, InvProjection);
	
	return viewSpace / viewSpace.w;
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

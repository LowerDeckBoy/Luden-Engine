#ifndef BINDLESS_HLSLI
#define BINDLESS_HLSLI

static Texture2D GetTexture(uint Index)
{
	return ResourceDescriptorHeap[Index];
}

template<typename T>
static RWTexture2D<float4> GetRWTexture(uint Index)
{
	return ResourceDescriptorHeap[Index];
}

template<typename T>
static StructuredBuffer<T> GetBuffer(uint Index)
{
	return ResourceDescriptorHeap[Index];
}

static float2 GetTexelSize(float2 TextureSize)
{
	return 1.0f / TextureSize;
}

// Get texture X and Y dimensions.
static float2 GetTextureSize(in Texture2D Texture)
{
	float2 dimensions;
	Texture.GetDimensions(dimensions.x, dimensions.y);
	
	return dimensions;
}

static float2 GetRWTextureSize(in RWTexture2D<float4> RWTexture)
{
	float2 dimensions;
	RWTexture.GetDimensions(dimensions.x, dimensions.y);
	
	return dimensions;
}

template<typename T>
static float2 GetTextureSize(in RWTexture2D<T> RWTexture)
{
	float2 dimensions;
	RWTexture.GetDimensions(dimensions.x, dimensions.y);
	
	return dimensions;
}

#endif // BINDLESS_HLSLI

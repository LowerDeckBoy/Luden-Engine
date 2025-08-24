#ifndef BINDLESS_HLSLI
#define BINDLESS_HLSLI

#define DECLARE_GET_TEXTURE2D(Type)

Texture2D GetTexture(uint Index)
{
	return ResourceDescriptorHeap[Index];
}

template<typename T>
RWTexture2D<float4> GetRWTexture(uint Index)
{
	return ResourceDescriptorHeap[Index];
}

template<typename T>
StructuredBuffer<T> GetBuffer(uint Index)
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

#endif // BINDLESS_HLSLI

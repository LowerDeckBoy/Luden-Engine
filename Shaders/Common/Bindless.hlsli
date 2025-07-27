#ifndef BINDLESS_HLSLI
#define BINDLESS_HLSLI

#define DECLARE_GET_TEXTURE2D(Type)

Texture2D GetTexture(uint Index)
{
	return ResourceDescriptorHeap[Index];
}

#endif // BINDLESS_HLSLI

#ifndef MIPMAP2D_HLSLI
#define MIPMAP2D_HLSLI

#include "MipMap_RS.hlsli"
#include "../Common/Bindless.hlsli"

struct MipmapParameters
{
	float2 	TexelSize;
	uint	SourceIndex;
	uint	DestinationIndex;
};

ConstantBuffer<MipmapParameters> Constants : register(b0);
SamplerState LinearClampSampler : register(s0);

[RootSignature(MIPMAP_RS)]
[numthreads(8, 8, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D 	source 		= GetTexture(Constants.SourceIndex);
	RWTexture2D destination = GetRWTexture<float4>(Constants.DestinationIndex);

	float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * Constants.TexelSize;
	destination[DispatchThreadID.xy] = source.Sample(LinearClampSampler, texCoord);
}

#endif // MIPMAP2D_HLSLI

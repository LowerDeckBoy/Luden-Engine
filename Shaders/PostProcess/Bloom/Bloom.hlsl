#ifndef BLOOM_HLSL
#define BLOOM_HLSL

#include "Common.hlsli"
#include "Bloom_RS.hlsli"

static const float Weights[5] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };

[RootSignature(BLOOM_ROOT_SIG)]
[numthreads(DISPATCH_GROUP, DISPATCH_GROUP, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4>	baseColorTexture	= ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D<float4>	lightingTexture		= ResourceDescriptorHeap[Constants.LightImageIndex];
	RWTexture2D<float4>	output				= ResourceDescriptorHeap[Constants.MipIndex];

	float2 textureSize;
	output.GetDimensions(textureSize.x, textureSize.y);
	
	const float2 texelSize = 1.0f / textureSize;
	const float2 texCoord = (DispatchThreadID.xy + 0.5f) * texelSize;
	
	output[DispatchThreadID.xy].a = 1.0f;
	output[DispatchThreadID.xy].rgb = lightingTexture.Sample(linearClampSampler, texCoord).rgb * Weights[0] * Constants.Intensity;

	for (uint i = 1; i < 5; ++i)
	{
		output[DispatchThreadID.xy].rgb += lightingTexture.Sample(linearClampSampler, texCoord + float2(texelSize.x * i, 0.0)).rgb * Weights[i].x * Constants.Intensity;
		output[DispatchThreadID.xy].rgb += lightingTexture.Sample(linearClampSampler, texCoord - float2(texelSize.x * i, 0.0)).rgb * Weights[i].x * Constants.Intensity;
	}
}

#endif // BLOOM_HLSL

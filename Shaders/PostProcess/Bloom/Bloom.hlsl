#ifndef BLOOM_HLSL
#define BLOOM_HLSL

#include "Common.hlsli"
#include "Bloom_RS.hlsli"
#include "../../Common/Color.hlsli"

float3 GetThreshold(float3 Color)
{
	float luminance = GetLuminance(Color);

	if (luminance < 1.0f)
	{
		return Color;
	}

	float soft = luminance - Constants.Threshold;
	//float soft = Constants.Threshold - luminance;
	soft = clamp(soft, 0.0f, Constants.ThresholdKnee);
	soft = soft * soft / (Constants.ThresholdKnee * 2.0f);

	float contribution = max(soft, luminance - Constants.Threshold);
	contribution /= max(luminance, 0.0001f);

	return Color * saturate(contribution);
}

static const float Weights[5] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };

[RootSignature(BLOOM_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> output 	= ResourceDescriptorHeap[Constants.MipIndex];
	const float2 textureSize 	= GetTextureSize(output);

	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}

	Texture2D<float4> emissiveTexture 	= GetTexture(Constants.EmissiveImageIndex);
	Texture2D<float4> lightingTexture 	= GetTexture(Constants.LightImageIndex);
		
	const float2 texelSize = GetTexelSize(textureSize);
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;

	float3 emissive = emissiveTexture.Load(uint3(DispatchThreadID.xy, 0)).rgb;
	float3 color 	= lightingTexture.Load(uint3(DispatchThreadID.xy, 0)).rgb;
	
	//output[DispatchThreadID.xy] = float4(emissive, 1.0f);

	for (uint i = 1; i < 5; ++i)
	{
		emissive += emissiveTexture.Sample(TexSampler, texCoord + float2(texelSize.x * i, 0.0f)).rgb * Weights[i] * Constants.Intensity;
		emissive += emissiveTexture.Sample(TexSampler, texCoord - float2(texelSize.x * i, 0.0f)).rgb * Weights[i] * Constants.Intensity;
		
		emissive += emissiveTexture.Sample(TexSampler, texCoord + float2(0.0f, texelSize.y * i)).rgb * Weights[i] * Constants.Intensity;
		emissive += emissiveTexture.Sample(TexSampler, texCoord - float2(0.0f, texelSize.y * i)).rgb * Weights[i] * Constants.Intensity;
	}
	
	emissive += GetThreshold(color);
	output[DispatchThreadID.xy] = float4(emissive, 1.0f);
}

#endif // BLOOM_HLSL

#ifndef BLUR_HLSL
#define BLUR_HLSL

#define DISPATCH_BLOCK 8

#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"
#include "Blur_RS.hlsli"

#define KERNEL_RADIUS 4

struct Parameters
{
	uint TargetImageIndex;
	uint DepthIndex;
	uint NormalIndex;
	float Sharpness;
};

static const float Weights[5] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };

ConstantBuffer<Parameters> Constants : register(b0);
SamplerState TexSampler : register(s0);

[RootSignature(BLUR_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4> sampleTexture = GetTexture(Constants.TargetImageIndex);
	//Texture2D depthTexture = GetTexture(Constants.DepthIndex);
	RWTexture2D<float4> targetTexture = GetRWTexture<float4>(Constants.TargetImageIndex);
	
	const float2 textureSize	= GetTextureSize(targetTexture);
	const float2 texelSize		= GetTexelSize(textureSize);
	const float2 texCoord		= (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	if (textureSize.x <= DispatchThreadID.x || textureSize.y <= DispatchThreadID.y)
	{
		return;
	}
	
	float3 source = sampleTexture.Sample(TexSampler, texCoord).xyz;
	//float depth = depthTexture.Sample(TexSampler, texCoord).r;

	float3 color = source;
	float blurRadius = 0.20f;
	float weightSum = 0.0f;
	for (int i = -KERNEL_RADIUS; i <= KERNEL_RADIUS; ++i)
	{
		float2 offset = float2(i * texelSize.x * blurRadius, 0.0f);
		float weight = Weights[abs(i)];
        
		color += sampleTexture.Sample(TexSampler, texCoord + offset).rgb * weight;
		weightSum += weight;
	}
	
	for (int j = -KERNEL_RADIUS; j <= KERNEL_RADIUS; ++j)
	{
		float2 offset = float2(j * texelSize.y * blurRadius, 0.0f);
		float weight = Weights[abs(j)];
        
		color += sampleTexture.Sample(TexSampler, texCoord + offset).rgb * weight;
		weightSum += weight;
	}
	
	targetTexture[DispatchThreadID.xy] = float4(color / weightSum, 1.0f);

}

#endif // BLUR_HLSL

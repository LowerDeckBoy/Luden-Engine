#ifndef BLUR_HLSL
#define BLUR_HLSL

#define DISPATCH_BLOCK 8

#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"
#include "Blur_RS.hlsli"

struct Parameters
{
	uint	TargetImageIndex;
	uint	SourceIndex;
	float	Sharpness;
	uint	Direction;
};

static const int BlurRadius = 2;
static const float Weights[5] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };

ConstantBuffer<Parameters> Constants : register(b0);
SamplerState TexSampler : register(s0);

[RootSignature(BLUR_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4>	sourceTexture	= GetTexture(Constants.SourceIndex);
	RWTexture2D<float4> targetTexture	= GetRWTexture<float4>(Constants.TargetImageIndex);
	
	const float2 textureSize	= GetTextureSize(targetTexture);
	const float2 texelSize		= GetTexelSize(textureSize);
	const float2 texCoord		= (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	if (textureSize.x <= DispatchThreadID.x || textureSize.y <= DispatchThreadID.y)
	{
		return;
	}
	
	float3 result = 0.0f.xxx;
	float blurSharpness = Constants.Sharpness;
	float weightSum = 0.0f;
	
	// Blur horizontally
	if (Constants.Direction == 0)
	{
		for (int i = -BlurRadius; i <= BlurRadius; ++i)
		{
			float2 offset = float2(i * texelSize.x * blurSharpness, 0.0f);
			float weight = Weights[abs(i)];
		
			result += sourceTexture.Sample(TexSampler, texCoord + offset).rgb * weight;
			weightSum += weight;
		}
	}
	// Blur vertically
	else
	{
		for (int j = -BlurRadius; j <= BlurRadius; ++j)
		{
			float2 offset = float2(0.0f, j * texelSize.y * blurSharpness);
			float weight = Weights[abs(j)];
		
			result += sourceTexture.Sample(TexSampler, texCoord + offset).rgb * weight;
			weightSum += weight;
		}
	}
	
	targetTexture[DispatchThreadID.xy] = float4(result / weightSum, 1.0f);
}

#endif // BLUR_HLSL

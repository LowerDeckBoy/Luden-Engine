#ifndef BLOOM_DOWNSAMPLE_HLSL
#define BLOOM_DOWNSAMPLE_HLSL

#include "Common.hlsli"

float3 Downsample(in Texture2D Texture, float2 UV, float2 TexelSize)
{
	const float x = TexelSize.x;
	const float y = TexelSize.y;

	const float3 a = Texture.Sample(TexSampler, float2(UV.x - 2.0f * x,		UV.y + 2.0f * y)).rgb;
	const float3 b = Texture.Sample(TexSampler, float2(UV.x,				UV.y + 2.0f * y)).rgb;
	const float3 c = Texture.Sample(TexSampler, float2(UV.x + 2.0f * x,		UV.y + 2.0f * y)).rgb;
	
	const float3 d = Texture.Sample(TexSampler, float2(UV.x - 2.0f * x, 	UV.y)).rgb;
	const float3 e = Texture.Sample(TexSampler, float2(UV.x,				UV.y)).rgb;
	const float3 f = Texture.Sample(TexSampler, float2(UV.x + 2.0f * x, 	UV.y)).rgb;
	
	const float3 g = Texture.Sample(TexSampler, float2(UV.x - 2.0f * x, 	UV.y - 2.0f * y)).rgb;
	const float3 h = Texture.Sample(TexSampler, float2(UV.x,				UV.y - 2.0f * y)).rgb;
	const float3 i = Texture.Sample(TexSampler, float2(UV.x + 2.0f * x, 	UV.y - 2.0f * y)).rgb;
	
	const float3 j = Texture.Sample(TexSampler, float2(UV.x - x,			UV.y + y)).rgb;
	const float3 k = Texture.Sample(TexSampler, float2(UV.x + x,			UV.y + y)).rgb;
	const float3 l = Texture.Sample(TexSampler, float2(UV.x - x,			UV.y - y)).rgb;
	const float3 m = Texture.Sample(TexSampler, float2(UV.x + x,			UV.y - y)).rgb;

	float3 result = e * 0.125f;
	result += (a + c + g + i) * 0.03125f;
	result += (b + d + f + h) * 0.0625f;
	result += (j + k + l + m) * 0.125f;

	return result;
}

[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4> sourceTexture 	= GetTexture(Constants.LightImageIndex);
	RWTexture2D<float4> output		 	= GetRWTexture<float4>(Constants.MipIndex);
	
	const float2 textureSize = GetTextureSize(output);
	
	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}

	const float2 texelSize	= GetTexelSize(textureSize);
	const float2 texCoord	= (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	//		  Texel
	//	-----------------
	//	a	-	b	-	c
	//
	//	-	j	-	k	-
	//
	//	d	-	e	-	f
	//
	//	-	l	-	m	-
	//
	//	g	-	h	-	i
	//	-----------------
	
	float3 result = Downsample(sourceTexture, texCoord, texelSize);
	output[DispatchThreadID.xy] = float4(result, 1.0f);
	
}

#endif // BLOOM_DOWNSAMPLE_HLSL

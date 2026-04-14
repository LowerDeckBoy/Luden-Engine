#ifndef BLOOM_UPSAMPLE_HLSL
#define BLOOM_UPSAMPLE_HLSL

#include "Common.hlsli"

float3 Upsample(in Texture2D Texture, float2 UV, float2 Texel)
{
	const float x = 0.05f;
	const float y = 0.05f;
	const float3 a = Texture.Sample(TexSampler, float2(UV.x - x,	UV.y + y)).rgb;
	const float3 b = Texture.Sample(TexSampler, float2(UV.x,		UV.y + y)).rgb;
	const float3 c = Texture.Sample(TexSampler, float2(UV.x + x,	UV.y + y)).rgb;
	
	const float3 d = Texture.Sample(TexSampler, float2(UV.x - x,	UV.y	)).rgb;
	const float3 e = Texture.Sample(TexSampler, float2(UV.x,		UV.y	)).rgb;
	const float3 f = Texture.Sample(TexSampler, float2(UV.x + x,	UV.y	)).rgb;
	
	const float3 g = Texture.Sample(TexSampler, float2(UV.x - x,	UV.y - y)).rgb;
	const float3 h = Texture.Sample(TexSampler, float2(UV.x,		UV.y - y)).rgb;
	const float3 i = Texture.Sample(TexSampler, float2(UV.x + x,	UV.y - y)).rgb;

	float3 result = e * 4.0f;
	result += (b + d + f + h) * 2.0f;
	result += (a + c + g + i);
	result *= 1.0f / 16.0f;

	return result;
}

[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4> sourceTexture = GetTexture(Constants.LightImageIndex);
	RWTexture2D<float4> output 		= GetRWTexture<float4>(Constants.MipIndex);
	
	const float2 textureSize  = GetTextureSize(output);
	
	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}

	const float2 texelSize 		= GetTexelSize(textureSize);
	const float2 texCoord 		= (float2(DispatchThreadID.xy) + 0.5f) * texelSize;

	//		  Texel
	//	-----------------
	//	a	-	b	-	c
	//
    //	d	-	e	-	f
	//
    //	g	-	h	-	i
	//	-----------------

	float3 result = Upsample(sourceTexture, texCoord, texelSize);

	output[DispatchThreadID.xy] = float4(result, 1.0f);
	
}

#endif // BLOOM_UPSAMPLE_HLSL

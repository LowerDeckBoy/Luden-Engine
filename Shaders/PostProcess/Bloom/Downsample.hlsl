#ifndef BLOOM_DOWNSAMPLE_HLSL
#define BLOOM_DOWNSAMPLE_HLSL

#include "Common.hlsli"

float3 Sample(Texture2D Texture, float2 UV, float Threshold)
{
	float3 color = Texture.Sample(linearClampSampler, UV).rgb;
	
	//float luminance = Luminance(color);
	//float a = saturate(luminance - Threshold);
	//color *= a * a;
	
	return color;
}

[numthreads(DISPATCH_GROUP, DISPATCH_GROUP, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	// Temporal index for testing only.
	Texture2D<float4> sourceTexture = ResourceDescriptorHeap[Constants.LightImageIndex];
	RWTexture2D<float4> output = ResourceDescriptorHeap[Constants.MipIndex];
	
	float2 textureSize;
	output.GetDimensions(textureSize.x, textureSize.y);

	const float2 texelSize = 1.0f / textureSize;
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	const float x = texelSize.x;
	const float y = texelSize.y;

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
	
	const float3 a = Sample(sourceTexture, float2(texCoord.x - 2.0f * x, texCoord.y + 2.0f * y), Constants.Threshold).rgb;
	const float3 b = Sample(sourceTexture, float2(texCoord.x, texCoord.y + 2.0f * y), Constants.Threshold).rgb;
	const float3 c = Sample(sourceTexture, float2(texCoord.x + 2.0f * x, texCoord.y + 2.0f * y), Constants.Threshold).rgb;
	
	const float3 d = Sample(sourceTexture, float2(texCoord.x - 2.0f * x, texCoord.y), Constants.Threshold).rgb;
	const float3 e = Sample(sourceTexture, float2(texCoord.x, texCoord.y), Constants.Threshold).rgb;
	const float3 f = Sample(sourceTexture, float2(texCoord.x + 2.0f * x, texCoord.y), Constants.Threshold).rgb;
	
	const float3 g = Sample(sourceTexture, float2(texCoord.x - 2.0f * x, texCoord.y - 2.0f * y), Constants.Threshold).rgb;
	const float3 h = Sample(sourceTexture, float2(texCoord.x, texCoord.y - 2.0f * y), Constants.Threshold).rgb;
	const float3 i = Sample(sourceTexture, float2(texCoord.x + 2.0f * x, texCoord.y - 2.0f * y), Constants.Threshold).rgb;
	
	const float3 j = Sample(sourceTexture, float2(texCoord.x - x, texCoord.y + y), Constants.Threshold).rgb;
	const float3 k = Sample(sourceTexture, float2(texCoord.x + x, texCoord.y + y), Constants.Threshold).rgb;
	const float3 l = Sample(sourceTexture, float2(texCoord.x - x, texCoord.y - y), Constants.Threshold).rgb;
	const float3 m = Sample(sourceTexture, float2(texCoord.x + x, texCoord.y - y), Constants.Threshold).rgb;
	
	
	
	
	/*
	const float3 a = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - 2.0f * x,	texCoord.y + 2.0f * y)).rgb;
	const float3 b = sourceTexture.Sample(linearClampSampler, float2(texCoord.x,			texCoord.y + 2.0f * y)).rgb;
	const float3 c = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + 2.0f * x,	texCoord.y + 2.0f * y)).rgb;
	
	const float3 d = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - 2.0f * x, texCoord.y)).rgb;
	const float3 e = sourceTexture.Sample(linearClampSampler, float2(texCoord.x,			texCoord.y)).rgb;
	const float3 f = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + 2.0f * x, texCoord.y)).rgb;
	
	const float3 g = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - 2.0f * x, texCoord.y - 2.0f * y)).rgb;
	const float3 h = sourceTexture.Sample(linearClampSampler, float2(texCoord.x,			texCoord.y - 2.0f * y)).rgb;
	const float3 i = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + 2.0f * x, texCoord.y - 2.0f * y)).rgb;
	
	const float3 j = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - x,		texCoord.y + y)).rgb;
	const float3 k = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + x,		texCoord.y + y)).rgb;
	const float3 l = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - x,		texCoord.y - y)).rgb;
	const float3 m = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + x,		texCoord.y - y)).rgb;
	*/
	
	float3 result = e * 0.125f;
	result += (a + c + g + i) * 0.03125f;
	result += (b + d + f + h) * 0.0625f;
	result += (j + k + l + m) * 0.125f;

	output[DispatchThreadID.xy] = float4(result, 1.0f);
	
}

#endif // BLOOM_DOWNSAMPLE_HLSL

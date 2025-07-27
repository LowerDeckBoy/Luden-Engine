#ifndef BLOOM_UPSAMPLE_HLSL
#define BLOOM_UPSAMPLE_HLSL

#include "Common.hlsli"

Texture2D<float4> GetTexture2D(uint Index)
{
	

	return ResourceDescriptorHeap[Index];
}

[numthreads(DISPATCH_GROUP, DISPATCH_GROUP, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	//Texture2D<float4> sourceTexture = ResourceDescriptorHeap[Constants.LightImageIndex];
	Texture2D<float4> sourceTexture = GetTexture2D(Constants.LightImageIndex);
	RWTexture2D<float4> output = ResourceDescriptorHeap[Constants.MipIndex];
	
	float filterRadius = 0.005f;
	
	float2 textureSize;
	output.GetDimensions(textureSize.x, textureSize.y);

	const float2 texelSize = 1.0f / textureSize;
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) / textureSize;

	const float x = filterRadius;
	const float y = filterRadius;
	
	//		  Texel
	//	-----------------
	//	a	-	b	-	c
	//
    //	d	-	e	-	f
	//
    //	g	-	h	-	i
	//	-----------------

	const float3 a = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - x,	texCoord.y + y)).rgb;
	const float3 b = sourceTexture.Sample(linearClampSampler, float2(texCoord.x,		texCoord.y + y)).rgb;
	const float3 c = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + x,	texCoord.y + y)).rgb;
	
	const float3 d = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - x,	texCoord.y)).rgb;
	const float3 e = sourceTexture.Sample(linearClampSampler, float2(texCoord.x,		texCoord.y)).rgb;
	const float3 f = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + x,	texCoord.y)).rgb;
	
	const float3 g = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - x,	texCoord.y - y)).rgb;
	const float3 h = sourceTexture.Sample(linearClampSampler, float2(texCoord.x,		texCoord.y - y)).rgb;
	const float3 i = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + x,	texCoord.y - y)).rgb;
	
	float3 result = e * 4.0f;
	result += (b + d + f + h) * 2.0;
	result += (a + c + g + i);
	result *= 1.0 / 16.0;
	
	float3 color = output[DispatchThreadID.xy].rgb;
	
	output[DispatchThreadID.xy] = float4(lerp(color, result, Constants.Gamma), 1.0f);
	
}

#endif // BLOOM_UPSAMPLE_HLSL

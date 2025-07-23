#ifndef BLOOM_HLSL
#define BLOOM_HLSL

#include "Common.hlsli"

#include "Bloom_RS.hlsli"

float4 Downsample(in Texture2D<float4> Source, uint2 TexCoords)
{
	float2 texelSize;
	Source.GetDimensions(texelSize.x, texelSize.y);
	texelSize = 1.0 / texelSize;
	
	const float3 a = Source.Sample(linearClampSampler, float2(TexCoords.x - 2 * texelSize.x, TexCoords.y + 2 * texelSize.y)).rgb;
	const float3 b = Source.Sample(linearClampSampler, float2(TexCoords.x,					 TexCoords.y + 2 * texelSize.y)).rgb;
	const float3 c = Source.Sample(linearClampSampler, float2(TexCoords.x + 2 * texelSize.x, TexCoords.y + 2 * texelSize.y)).rgb;

	const float3 d = Source.Sample(linearClampSampler, float2(TexCoords.x - 2 * texelSize.x, TexCoords.y)).rgb;
	const float3 e = Source.Sample(linearClampSampler, float2(TexCoords.x,					 TexCoords.y)).rgb;
	const float3 f = Source.Sample(linearClampSampler, float2(TexCoords.x + 2 * texelSize.x, TexCoords.y)).rgb;

	const float3 g = Source.Sample(linearClampSampler, float2(TexCoords.x - 2 * texelSize.x, TexCoords.y - 2 * texelSize.y)).rgb;
	const float3 h = Source.Sample(linearClampSampler, float2(TexCoords.x,					 TexCoords.y - 2 * texelSize.y)).rgb;
	const float3 i = Source.Sample(linearClampSampler, float2(TexCoords.x + 2 * texelSize.x, TexCoords.y - 2 * texelSize.y)).rgb;

	const float3 j = Source.Sample(linearClampSampler, float2(TexCoords.x - texelSize.x, TexCoords.y + texelSize.y)).rgb;
	const float3 k = Source.Sample(linearClampSampler, float2(TexCoords.x + texelSize.x, TexCoords.y + texelSize.y)).rgb;
	const float3 l = Source.Sample(linearClampSampler, float2(TexCoords.x - texelSize.x, TexCoords.y - texelSize.y)).rgb;
	const float3 m = Source.Sample(linearClampSampler, float2(TexCoords.x + texelSize.x, TexCoords.y - texelSize.y)).rgb;
	
	float3 color = e * 0.125;
	color += (a + c + g + i) * 0.03125;
	color += (b + d + f + h) * 0.0625;
	color += (j + k + l + m) * 0.125;
	
	return float4(color, 1.0);
}

float4 Upsample();

float4 Combine();

static float Weights[5] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };

// https://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare/
// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
// https://github.com/PuddingCoke/Direct3D-12-toy-engine/blob/master/Engine/Shaders/Bloom/BloomHBlurCS.hlsl

[RootSignature(BLOOM_ROOT_SIG)]
[numthreads(32, 32, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4> baseColorTexture = ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D<float4> lightingTexture = ResourceDescriptorHeap[Constants.LightImageIndex];
	// Just test
	RWTexture2D<float4> output = ResourceDescriptorHeap[Constants.SceneImageIndex];
	
	float4 lighting = lightingTexture[DispatchThreadID.xy];
	float4 baseColor = baseColorTexture[DispatchThreadID.xy];
	
	float2 textureSize;
	output.GetDimensions(textureSize.x, textureSize.y);
	
	float2 texOffset = 1.0f / textureSize;
	
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * texOffset;
	
	output[DispatchThreadID.xy].a = 1.0f;
	output[DispatchThreadID.xy].rgb = lightingTexture.Sample(linearClampSampler, texCoord).rgb * Weights[0] * Constants.Intensity;

	for (uint i = 1; i < 5; ++i)
	{
		output[DispatchThreadID.xy].rgb += lightingTexture.Sample(linearClampSampler, texCoord + float2(texOffset.x * i, 0.0)).rgb * Weights[i].x * Constants.Intensity;
		output[DispatchThreadID.xy].rgb += lightingTexture.Sample(linearClampSampler, texCoord - float2(texOffset.x * i, 0.0)).rgb * Weights[i].x * Constants.Intensity;
	}
}

#endif // BLOOM_HLSL

#ifndef BLOOM_HLSL
#define BLOOM_HLSL

#include "../../Common/Bindless.hlsli"
#include "Common.hlsli"
#include "Bloom_RS.hlsli"
#include "../../Common/Color.hlsli"

float3 KarisAverage(float3 A, float3 B, float3 C, float3 D)
{
	// See: https://graphicrants.blogspot.com/2013/12/tone-mapping.html
	
	float3 weightA = 1.f / (1.f + GetLuminance(A));
	float3 weightB = 1.f / (1.f + GetLuminance(B));
	float3 weightC = 1.f / (1.f + GetLuminance(C));
	float3 weightD = 1.f / (1.f + GetLuminance(D));
	
	return (A * weightA + B * weightB + C * weightC + D * weightD) / (weightA + weightB + weightC + weightD);
}

float3 GetThreshold(float3 Color)
{
	float luminance = GetLuminance(Color);

	if (luminance < Constants.Threshold)
	{
		return float3(0.0f, 0.0f, 0.0f);
	}

	float soft = luminance - Constants.Threshold;
	soft = clamp(soft, 0.0f, Constants.ThresholdKnee);
	soft = soft * soft / (Constants.ThresholdKnee * 2.0f);

	float contribution = max(soft, luminance - Constants.Threshold);
	contribution /= max(luminance, 0.0001f);

	return Color * saturate(contribution);
}

float3 GetThreshold_TEST(float3 Color)
{
	float luminance = GetLuminance(Color);

	//if (luminance < Constants.Threshold)
	//{
	//	return float3(0.0f, 0.0f, 0.0f);
	//}

	const float MaxBrightness = 10.0f;

	float3 output = min(Color, MaxBrightness);
	float soft = luminance - Constants.Threshold + Constants.ThresholdKnee;

	soft = clamp(soft, 0.0f, 2.0f * Constants.ThresholdKnee);
	soft = soft * soft / (4.0f * Constants.ThresholdKnee + FLOAT_MIN);

	float contribution = max(soft, luminance - Constants.Threshold);
	contribution /= max(luminance, FLOAT_MIN);
	output *= contribution;
	output /= (1.0f + luminance);

	return saturate(output);
}

static const float Weights[5] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };

[RootSignature(BLOOM_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> output 	= GetRWTexture<float4>(Constants.MipIndex);
	const float2 textureSize 	= GetTextureSize(output);

	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}

	Texture2D<float4> emissiveTexture 	= GetTexture(Constants.EmissiveImageIndex);
	Texture2D<float4> lightingTexture 	= GetTexture(Constants.LightImageIndex);
		
	const float2 texelSize = GetTexelSize(textureSize);
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;

	float3 emissive = emissiveTexture.Load(uint3(DispatchThreadID.xy, 0)).rgb * Constants.Intensity;
	float3 color 	= lightingTexture.Load(uint3(DispatchThreadID.xy, 0)).rgb;

	//emissive += GetThreshold(color);
	emissive += GetThreshold_TEST(color);
	output[DispatchThreadID.xy] = float4(emissive, 1.0f);
}

#endif // BLOOM_HLSL

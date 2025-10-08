#ifndef SSAO_HLSL
#define SSAO_HLSL

#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"
#include "SSAO_RS.hlsli"

#define DISPATCH_BLOCK 8

const static uint KernelSize = 64;

struct SSAOParameters
{
	float4x4 Projection;
	float4x4 InvProjection;
	float4x4 View;
	float4x4 InvView;

	uint OutputImageIndex;
	uint BaseColorIndex;
	uint NormalIndex;
	uint WorldPositionIndex;
	
	float Radius;
	float Power;
	float Bias;
	uint pad2;
	
	float4 Samples[64];
};

ConstantBuffer<SSAOParameters> Constants : register(b0);
SamplerState LinearBorderSampler : register(s0);
SamplerState PointWrapSampler : register(s1);
SamplerState TexSampler : register(s2);

// https://stackoverflow.com/questions/55530121/why-is-ssao-only-working-from-certain-angles-distances
// https://github.com/SaschaWillems/Vulkan/blob/master/shaders/hlsl/ssao/ssao.frag
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/ssao/ssao.cpp

[RootSignature(SSAO_ROOT_SIG)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> output = GetRWTexture<float4>(Constants.OutputImageIndex);
	
	float2 textureSize 	= GetTextureSize(output);
	float2 texelSize 	= GetTexelSize(textureSize);
	float2 texCoord 	= (float2(DispatchThreadID.xy) + 0.5f) * texelSize;

	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}
	
	Texture2D<float4> texNormal			= GetTexture(Constants.NormalIndex);
	Texture2D<float4> texWorldPosition	= GetTexture(Constants.WorldPositionIndex);
	Texture2D<float4> texNoise			= GetTexture(Constants.BaseColorIndex);

	float3 N = (texNormal.Sample(TexSampler, texCoord).rgb);

	float depth = texWorldPosition.Sample(TexSampler, texCoord).w;
	float3 viewPosition = GetViewPosition(texCoord, depth, Constants.InvProjection);
	//output[DispatchThreadID.xy] = float4(viewPosition, 1.0f);
	//return;
	float2 noiseDimensions = GetTextureSize(texNoise);
	float2 noiseScale = textureSize / noiseDimensions;
	float3 randomVector = normalize(float3(texNoise.Sample(LinearBorderSampler, texCoord * noiseScale).xyz * 2.0f - 1.0f));
	output[DispatchThreadID.xy] = float4(randomVector, 1.0f);
	return;
	float3 tangent 		= normalize(randomVector - N * dot(randomVector, N));
	float3 bitangent	= cross(N, tangent);
	float3x3 TBN		= float3x3(tangent, bitangent, N);
	
	float occlusion = 0.0f;
	for (int i = 0; i < KernelSize; ++i)
	{
		float3 sampleDir = mul(Constants.Samples[i].xyz, TBN);
		float3 samplePos = viewPosition + sampleDir * Constants.Radius;
		
		float4 offset = float4(samplePos, 1.0f);
		offset = mul(offset, Constants.Projection);
		offset.xy /= offset.w;
		offset.xy = float2(offset.xy * float2(0.5f, 0.5f) + float2(0.5f, 0.5f));
		offset.y *= -1.0f;
		
		float sampledDepth = texWorldPosition.Sample(TexSampler, offset.xy).w;
		sampledDepth = GetViewPosition(offset.xy, sampledDepth, Constants.InvProjection).z;

		float rangeCheck = smoothstep(0.0f, 1.0f, Constants.Radius / abs(viewPosition.z - sampledDepth));
		occlusion += step(sampledDepth, samplePos.z - Constants.Bias) * rangeCheck;
	}
	
	occlusion = 1.0f - (occlusion / (float)KernelSize);
	occlusion = pow(abs(occlusion), Constants.Power);
	output[DispatchThreadID.xy] = float4(occlusion, occlusion, occlusion, 1.0f);

}

#endif // SSAO_HLSL

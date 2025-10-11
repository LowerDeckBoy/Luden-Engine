#ifndef SSAO_HLSL
#define SSAO_HLSL

#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"
#include "SSAO_RS.hlsli"

#define DISPATCH_BLOCK 8

const static uint KernelSize = 32;

struct SSAOParameters
{
	float4x4 Projection;
	float4x4 InvProjection;

	uint OutputImageIndex;
	uint NoiseIndex;
	uint NormalVSIndex;
	uint DepthIndex;
	
	float Radius;
	float Power;
	float Bias;
	uint  padding;
	
	float4 Samples[KernelSize];
};

ConstantBuffer<SSAOParameters> Constants : register(b0);
SamplerState LinearWrapSampler : register(s0);

[RootSignature(SSAO_ROOT_SIG)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> output = GetRWTexture<float4>(Constants.OutputImageIndex);
	
	const float2 textureSize = GetTextureSize(output);
	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}
	
	const float2 texelSize = GetTexelSize(textureSize);
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	Texture2D<float4> texNormal		= GetTexture(Constants.NormalVSIndex);
	Texture2D<float4> texNoise		= GetTexture(Constants.NoiseIndex);
	Texture2D<float4> texDepth		= GetTexture(Constants.DepthIndex);

	const float3 normal			= (texNormal.Sample(LinearWrapSampler, texCoord).rgb * 2.0f - 1.0f);
	const float  depth			= texDepth.Sample(LinearWrapSampler, texCoord).x;
	const float3 viewPosition	= GetViewPosition(texCoord, depth, Constants.InvProjection);
	
	const float2 noiseDimensions = GetTextureSize(texNoise);
	const float2 noiseScale		 = textureSize / noiseDimensions;
	const float3 randomVector	 = normalize(float3(texNoise.Sample(LinearWrapSampler, texCoord * noiseScale).xy * 2.0f - 1.0f, 0.0f));
	
	const float3 tangent	= normalize(randomVector - normal * dot(randomVector, normal));
	const float3 bitangent	= cross(normal, tangent);
	const float3x3 TBN		= float3x3(tangent, bitangent, normal);
	
	float occlusion = 0.0f;
	for (int i = 0; i < KernelSize; ++i)
	{
		const float3 sampleDir = mul(Constants.Samples[i].xyz, TBN);
		const float3 samplePos = viewPosition + sampleDir * Constants.Radius;
		
		float4 offset = float4(samplePos, 1.0f);
		offset = mul(offset, Constants.Projection);
		offset.xy /= offset.w;
		offset.xy = float2(offset.xy * float2(0.5f, 0.5f) + float2(0.5f, 0.5f));
		offset.y = 1.0f - offset.y;
		
		float sampledDepth = texDepth.Sample(LinearWrapSampler, offset.xy).x;
		sampledDepth = GetViewPosition(offset.xy, sampledDepth, Constants.InvProjection).z;

		const float rangeCheck = smoothstep(0.0f, 1.0f, Constants.Radius / abs(viewPosition.z - sampledDepth));
		occlusion += step(sampledDepth, samplePos.z - Constants.Bias) * rangeCheck;
	}
	
	occlusion = 1.0f - (occlusion / (float)KernelSize);
	occlusion = pow(abs(occlusion), Constants.Power);
	output[DispatchThreadID.xy] = float4(occlusion, occlusion, occlusion, 1.0f);

}

#endif // SSAO_HLSL

#ifndef SSAO_HLSL
#define SSAO_HLSL

#include "../Common/Common.hlsli"
#include "SSAO_RS.hlsli"

const static uint KernelSize = 64;

struct SSAOParameters
{
	 float4x4 Projection;
	row_major float4x4 InvViewProjection;

	uint OutputImageIndex;
	uint BaseColorIndex;
	uint NormalIndex;
	uint WorldPositionIndex;
	
	float Radius;
	float Bias;
	uint pad;
	uint pad2;
	
	float4 Samples[64];
	float4 Noise[16];
};

ConstantBuffer<SSAOParameters> Constants : register(b0);
SamplerState texSampler : register(s0);

float2 hash2(inout float HASH2SEED)
{
	HASH2SEED += 0.1f;
	float2 x = frac(sin(float2(HASH2SEED, HASH2SEED + 0.1f)) * float2(43758.5453123, 22578.1459123));
	HASH2SEED += 0.2f;
	return x;
}

[RootSignature(SSAO_ROOT_SIG)]
[numthreads(16, 16, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	const uint2 uv = DispatchThreadID.xy;

	RWTexture2D<float4> output = ResourceDescriptorHeap[Constants.OutputImageIndex];
	
	float2 textureSize;
	output.GetDimensions(textureSize.x, textureSize.y);
	const float2 texelSize = 1.0f / textureSize;
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	Texture2D<float3> texBaseColor		= ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D<float4> texNormal			= ResourceDescriptorHeap[Constants.NormalIndex];
	Texture2D<float4> texWorldPosition	= ResourceDescriptorHeap[Constants.WorldPositionIndex];

	//float4 normal = texNormal.Load(int3(uv, 0.0f));
	float4 normal = texNormal.Load(int3(uv, 0.0f));
	const float3 N = normal.rgb;

	const float3 worldPosition = texWorldPosition.Load(int3(uv, 0.0f)).rgb;
	//output[DispatchThreadID.xy] = float4(worldPosition, 1.0f);
	//return;

	// Test
	float seed;
	seed = (texCoord.x * texCoord.y) * textureSize.y;
	float3 randomVec = float3(hash2(seed), hash2(seed).x);
	
	 // Create TBN
	float3 tangent = normalize(randomVec - N * dot(randomVec, N));
	float3 bitangent = cross(N, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, N);
	
	
	float occlusion = 0.0f;
	for (int i = 0; i < 64; ++i)
	{
		//float3 samplePos = worldPosition + Constants.Samples[i].xyz;
		//float3 samplePos = N * Constants.Samples[i].xyz;
		float3 samplePos = mul((float3x3)TBN, Constants.Samples[i].xyz);
		//float3 samplePos = N * Constants.Samples[i].xyz;
		samplePos = worldPosition + samplePos * Constants.Radius; // 
	
		float4 offset = float4(samplePos, 1.0f);
		offset = mul(Constants.Projection, offset);
		//offset = mul(offset, Constants.Projection);
		offset.xy /= offset.w;
		//offset.xy = offset.xy * 0.5f + float2(0.5f, 0.5f);
		offset.x = offset.x * 0.5f + 0.5f;
		offset.y = -offset.y * 0.5f + 0.5f;
		
		//float Depth = texBaseColor.Sample(texSampler, offset.xy).x;

	//	float Depth = texWorldPosition.Sample(texSampler, offset.xy).w;
		float Depth = texWorldPosition.Sample(texSampler, offset.xy).w;
		float rangeCheck = smoothstep(0.0f, 1.0f, Constants.Radius / abs(worldPosition.z - Depth));
		occlusion += (Depth >= samplePos.z + Constants.Bias ? 1.0f : 0.0f) * rangeCheck;
	}
	
	occlusion = 1.0f - (occlusion / 64.0f);
	occlusion = pow(occlusion, 2.0f);
	output[DispatchThreadID.xy] = float4(occlusion, occlusion, occlusion, 1.0f);

}

#endif // SSAO_HLSL

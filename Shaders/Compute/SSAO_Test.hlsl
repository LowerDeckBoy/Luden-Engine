#ifndef SSAO_TEST_HLSL
#define SSAO_TEST_HLSL

#include "../Common/Common.hlsli"
#include "SSAO_RS.hlsli"

const static uint KernelSize = 64;

struct SSAOParameters
{
	float4x4 Projection;
	float4x4 InvProjection;
	float4x4 InvView;
	float4x4 InvViewProjection;

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

struct ScreenQuadOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

ConstantBuffer<SSAOParameters> Constants : register(b0);

SamplerState texSampler : register(s0);
SamplerState texNoiseSampler : register(s1);

float2 hash2(inout float HASH2SEED)
{
	HASH2SEED += 0.1f;
	float2 x = frac(sin(float2(HASH2SEED, HASH2SEED + 0.1f)) * float2(43758.5453123, 22578.1459123));
	HASH2SEED += 0.2f;
	return x;
}

float3 WorldPosFromDepth(float depth, float2 UV)
{
	//float z = depth * 2.0f - 1.0f;
	float z = depth;

	float4 clipSpacePosition = float4(UV * 2.0f - 1.0f, z, 1.0f);
	
	//float4 clipSpacePosition = float4(UV, z, 1.0f);
	//float4 viewSpacePosition = mul(Constants.InvProjection, clipSpacePosition);
	float4 viewSpacePosition = mul(clipSpacePosition, Constants.InvProjection);

    // Perspective division
	viewSpacePosition /= viewSpacePosition.w;
	
	//float4 worldSpacePosition = mul(Constants.InvView, viewSpacePosition);
	float4 worldSpacePosition = mul(viewSpacePosition, Constants.InvView);
	
	return worldSpacePosition.xyz;
	
	/*
	Texture2D<float4> output = ResourceDescriptorHeap[Constants.OutputImageIndex];
	
	float2 textureSize;
	output.GetDimensions(textureSize.x, textureSize.y);
	
		//https://www.gamedev.net/forums/topic/712179-how-can-i-restore-world-coordinates-in-hlsl-by-depth-value-and-screen-space-coordinates/
	float4 ndcCoords = float4(
        UV.x / textureSize.x * 2.0f - 1.0f,
        UV.y / textureSize.y * (2.0f) + 1.0f,
        depth ,
        1.0);

	//float4 worldCoords = mul(transpose(Constants.InvViewProjection), ndcCoords);
	float4 worldCoords = mul(ndcCoords, Constants.InvViewProjection);

	return worldCoords.xyz / worldCoords.w;
	*/
}


[RootSignature(SSAO_ROOT_SIG)]
ScreenQuadOutput VSMain(uint VertexID : SV_VertexID)
{
	ScreenQuadOutput output = (ScreenQuadOutput) 0;
	
	output.TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	output.Position = float4(output.TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
	output.Position.y *= -1.0f;
	
	return output;
}

float4 PSMain(ScreenQuadOutput pin) : SV_TARGET0
{
	//const uint2 uv = pin.TexCoord.xy;

	Texture2D<float4> output = ResourceDescriptorHeap[Constants.OutputImageIndex];
	
	float2 textureSize;
	output.GetDimensions(textureSize.x, textureSize.y);

	Texture2D<float4> texNoise			= ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D<float4> texNormal			= ResourceDescriptorHeap[Constants.NormalIndex];
	Texture2D<float4> texWorldPosition	= ResourceDescriptorHeap[Constants.WorldPositionIndex];

	float4 normal = normalize(texNormal.Load(int3(pin.Position.xy, 0.0f)));
	 float3 N = normal.rgb;
	//return float4(N, 1.0f);
	
	//float2 uv = TexelToUV()
	
	float3 worldPosition = texWorldPosition.Load(int3(pin.Position.xy, 0.0f)).xyz;
	float depth = -texWorldPosition.Load(int3(pin.Position.xy, 0.0f)).w;
	//float3 worldPosition = WorldPosFromDepth(depth, pin.TexCoord.xy).xyz;
	worldPosition = ClipToViewSpace(float4(worldPosition, 1.0f), Constants.InvProjection).rgb;
	//worldPosition = mul((float3x3)Constants.InvView, worldPosition);
	//worldPosition = mul(worldPosition, (float3x3)Constants.InvView);
	//return float4(worldPosition, 1.0f);

	// Test
	float seed;
	seed = (pin.TexCoord.x * pin.TexCoord.y) * (textureSize.y);
	float3 randomVec = normalize(float3(hash2(seed), hash2(seed).x));
	//return float4(randomVec , 1.0f);

	float2 noiseSize;
	texNoise.GetDimensions(noiseSize.x, noiseSize.y);
	noiseSize /= 4.0f;
	//float3 noise = texNoise.Load(int3(pin.Position.xy, 0.0f)).rgb;
	float3 noise = texNoise.Sample(texNoiseSampler, pin.TexCoord.xy * 4.0f).rgb;
	//return float4(noise, 1.0f);
	 // Create TBN
	//N = N * 2.0f - 1.0f;
	float3 tangent = normalize(noise - N * dot(noise, N));
	float3 bitangent = cross(N, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, N);
	//TBN = transpose(TBN);
	float occlusion = 0.0f;
	for (int i = 0; i < 64; ++i)
	{
		float3 samplePos = mul((float3x3) transpose(TBN), Constants.Samples[i].xyz);
		samplePos = worldPosition + samplePos * Constants.Radius;
		//float3 samplePos = mul(Constants.Samples[i].xyz, (float3x3) TBN);
		//float3 samplePos = mul(N, Constants.Samples[i].xyz);
		//samplePos = samplePos * Constants.Radius + worldPosition;
		//samplePos = ClipToViewSpace(float4(samplePos, 1.0f), Constants.InvProjection).xyz;
		//samplePos = mul((float3x3)Constants.InvView, samplePos);
		//samplePos = mul(samplePos, (float3x3)Constants.InvView);
		//samplePos = mul((float3x3) Constants.InvViewProjection, samplePos);
		//samplePos = mul((float3x3) Constants.InvView, samplePos);
		
		//samplePos = samplePos * worldPosition;

		float4 offset = float4(samplePos, 1.0f);
		offset = mul(Constants.InvViewProjection, offset);
		offset.xy /= offset.w;
		//offset.xy = offset.xy * float2(0.5f, -0.5f) + 0.5f;

		
		float sampledDepth = texWorldPosition.Sample(texSampler, offset.xy).w;
		//float sampledDepth = texWorldPosition.Load(uint3(offset.xy, 0.0f)).w;
		float rangeCheck = smoothstep(0.0f, 1.0f, Constants.Radius / abs(worldPosition.z - sampledDepth));
		occlusion += (sampledDepth >= samplePos.z + Constants.Bias ? 1.0f : 0.0f) * rangeCheck; // 
		//occlusion += (sampledDepth <= samplePos.z + Constants.Bias ? 1.0f : 0.0f) * rangeCheck; // 
		//occlusion += (sampledDepth >= worldPosition.z + Constants.Bias ? 1.0f : 0.0f) * rangeCheck; // 
	}
	
	occlusion = 1.0f - (occlusion / 64.0f);
	occlusion = pow(occlusion, 2.0f);
	return float4(occlusion, occlusion, occlusion, 1.0f);
}

#endif // SSAO_TEST_HLSL

#ifndef DEFERRED_HLSL
#define DEFERRED_HLSL

#include "Deferred_RS.hlsli"

struct PushConstants
{
	uint LightBufferIndex;
	uint NumLights;
	uint BaseColorIndex;
	uint NormalIndex;
	uint MRIndex;
	uint EmissiveIndex;
};

ConstantBuffer<PushConstants> Constants : register(b2);

struct ScreenQuadOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

ScreenQuadOutput VSMain(uint VertexID : SV_VertexID)
{
	ScreenQuadOutput output = (ScreenQuadOutput) 0;
	
	output.TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	output.Position = float4(output.TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
	output.Position.y *= -1.0f;
	
	return output;
}

[RootSignature(ROOT_SIG)]
float4 PSMain(ScreenQuadOutput pin) : SV_TARGET0
{
	float3 output = float3(1.0f, 1.0f, 1.0f);

	const float2 uv = pin.Position.xy;
	
	Texture2D texBaseColor	= ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D texNormal		= ResourceDescriptorHeap[Constants.NormalIndex];
	Texture2D texMR			= ResourceDescriptorHeap[Constants.MRIndex];
	Texture2D texEmissive	= ResourceDescriptorHeap[Constants.EmissiveIndex];
	
	const float4 baseColor			= texBaseColor.Load(int3(uv, 0.0f));
	const float3 normal				= texNormal.Load(int3(uv, 0.0f)).rgb;
	const float3 metallicRoughness	= texMR.Load(int3(uv, 0.0f)).rgb;
	const float3 emissive			= texEmissive.Load(int3(uv, 0.0f)).rgb;
	
	const float metalness			= metallicRoughness.b;
	const float roughness			= metallicRoughness.g;
	
	return float4(baseColor.rgb, 1.0f);
	return float4(pin.TexCoord, 0.0f, 1.0f);
}

#endif // DEFERRED_HLSL

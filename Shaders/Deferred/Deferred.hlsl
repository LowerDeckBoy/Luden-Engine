#ifndef DEFERRED_HLSL
#define DEFERRED_HLSL

#include "Deferred_RS.hlsli"
#include "../PBR.hlsli"
#include "../Common/Light.hlsli"
#include "../Common/Common.hlsli"
#include "../Common/Scene.hlsli"

struct PushConstants
{
	uint PointLightBufferIndex;
	uint NumPointLights;
	uint SpotLightBufferIndex;
	uint NumSpotLights;
	uint BaseColorIndex;
	uint NormalIndex;
	uint MRIndex;
	uint EmissiveIndex;
	uint WorldPositionIndex;
	uint padding;
};

ConstantBuffer<PushConstants> Constants : register(b2);

ConstantBuffer<SceneConstants> Scene : register(b1);

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

[earlydepthstencil]
[RootSignature(DEFERRED_RS)]
float4 PSMain(ScreenQuadOutput pin) : SV_TARGET0
{
	const float2 uv = pin.Position.xy;
	
	Texture2D<float4> texBaseColor		= ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D<float4> texNormal			= ResourceDescriptorHeap[Constants.NormalIndex];
	Texture2D<float4> texMR				= ResourceDescriptorHeap[Constants.MRIndex];
	Texture2D<float4> texEmissive		= ResourceDescriptorHeap[Constants.EmissiveIndex];
	Texture2D<float4> texWorldPositon	= ResourceDescriptorHeap[Constants.WorldPositionIndex];
	
	//const float4 baseColor			= pow(texBaseColor.Load(int3(uv, 0.0f)), 2.2f);
	const float4 baseColor			= texBaseColor.Load(int3(uv, 0.0f));
	const float4 normal				= texNormal.Load(int3(uv, 0.0f));
	const float3 metallicRoughness	= texMR.Load(int3(uv, 0.0f)).rgb;
	const float3 emissive			= texEmissive.Load(int3(uv, 0.0f)).rgb;
	const float3 worldPosition		= texWorldPositon.Load(int3(uv, 0.0f)).rgb;
	
	const float metalness			= metallicRoughness.b;
	const float roughness			= metallicRoughness.g;
	
	float3 albedo = baseColor.rgb + emissive.rgb;

	const float3 N = normalize(normal.rgb);
	const float  depth = normal.w;
	
	const float3 V = normalize(Scene.CameraPosition - worldPosition);
	const float NdotV = max(dot(N, V), Epsilon);
	
	const float3 Fdielectric = float3(0.04f, 0.04f, 0.04f);
	const float3 F0 = lerp(Fdielectric, baseColor.rgb, float3(metalness, metalness, metalness));
	
	float3 output = float3(0.0f, 0.0f, 0.0f);

	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	
	// Point lights
	StructuredBuffer<PointLight> PointLights = ResourceDescriptorHeap[Constants.PointLightBufferIndex];
	for (uint pointLightIdx = 0; pointLightIdx < Constants.NumPointLights; ++pointLightIdx)
	{
		PointLight light = PointLights[pointLightIdx];
		Lo += CalculatePointLight(light, baseColor.rgb, N, V, NdotV, worldPosition, metalness, roughness);
	}
	
	// Spot lights
	StructuredBuffer<SpotLight> SpotLights = ResourceDescriptorHeap[Constants.SpotLightBufferIndex];
	for (uint spotLightIdx = 0; spotLightIdx < Constants.NumSpotLights; ++spotLightIdx)
	{
		SpotLight light = SpotLights[spotLightIdx];
		Lo += CalculateSpotLight(light, baseColor.rgb, N, V, NdotV, worldPosition, metalness, roughness);
	}
	
	// Single directional lighting.
	{
		Lo += CalculateDirectionalLight(Scene.DirectionalPosition, Scene.DirectionalAmbient, Scene.DirectionalIntensity, baseColor.rgb, N, V, NdotV, metalness, roughness);
	}
	
	output += Lo;
	output += emissive;

	return float4(output.rgb, 1.0f);
}

#endif // DEFERRED_HLSL

#ifndef DEFERRED_CS_HLSL
#define DEFERRED_CS_HLSL

#include "Deferred_RS.hlsli"
#include "../PBR.hlsli"
#include "../Common/Bindless.hlsli"
#include "../Common/Light.hlsli"
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
	uint TextureOutputIndex;
	uint padding;
};

ConstantBuffer<SceneConstants>	Scene		: register(b1);
ConstantBuffer<PushConstants>	Constants	: register(b2);
SamplerState					texSampler	: register(s0);

[RootSignature(DEFERRED_RS)]
[numthreads(8, 8, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> outputTexture = GetRWTexture<float4>(Constants.TextureOutputIndex);

	float2 textureSize = GetTextureSize(outputTexture);
	if (textureSize.x < DispatchThreadID.x || textureSize.y < DispatchThreadID.y)
	{
		return;
	}

	Texture2D texBaseColor		= GetTexture(Constants.BaseColorIndex);
	Texture2D texNormal			= GetTexture(Constants.NormalIndex);
	Texture2D texMR				= GetTexture(Constants.MRIndex);
	Texture2D texEmissive		= GetTexture(Constants.EmissiveIndex);
	Texture2D texWorldPositon	= GetTexture(Constants.WorldPositionIndex);

	const float3 uv = int3(DispatchThreadID.xy, 0.0f);
	
	const float4 baseColor			= texBaseColor.Load(uv);
	const float4 normal				= texNormal.Load(uv);
	const float3 metallicRoughness	= texMR.Load(uv).rgb;
	const float3 emissive			= texEmissive.Load(uv).rgb;
	const float3 worldPosition		= texWorldPositon.Load(uv).rgb;
	
	const float metalness = metallicRoughness.b;
	const float roughness = metallicRoughness.g;
	
	const float3 N = normalize(normal.rgb);
	const float depth = normal.w;
	
	const float3 V = normalize(Scene.CameraPosition - worldPosition);
	const float NdotV = max(dot(N, V), Epsilon);

	const float3 F0 = lerp(Fdielectric, baseColor.rgb, float3(metalness, metalness, metalness));
	
	float3 output = float3(0.0f, 0.0f, 0.0f);

	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	
	// Test
	float3 scattering = float3(0.0f, 0.0f, 0.0f);
	
	// Point lights
	StructuredBuffer<PointLight> PointLights = GetBuffer<PointLight>(Constants.PointLightBufferIndex);
	for (uint pointLightIdx = 0; pointLightIdx < Constants.NumPointLights; pointLightIdx++)
	{
		PointLight light = PointLights[pointLightIdx];
		Lo += CalculatePointLight(light, baseColor.rgb, N, V, NdotV, worldPosition, metalness, roughness);
	}
	
	// Spot lights
	StructuredBuffer<SpotLight> SpotLights =  GetBuffer<SpotLight>(Constants.SpotLightBufferIndex);
	for (uint spotLightIdx = 0; spotLightIdx < Constants.NumSpotLights; spotLightIdx++)
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
	
	outputTexture[DispatchThreadID.xy] = float4(output.rgb, 1.0f);
	
}

#endif // DEFERRED_CS_HLSL

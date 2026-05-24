#ifndef TRANSPARENCY_PS_HLSL
#define TRANSPARENCY_PS_HLSL

#include "../Mesh/GBufferCommon.hlsli"
#include "../Common/Bindless.hlsli"
#include "../Common/Light.hlsli"
#include "../Common/Scene.hlsli"

ConstantBuffer<SceneConstants> Scene : register(b3, space0);

SamplerState AnisotropicSampler : register(s0);

static int IsIndexValid(uint Index)
{
	if (Index == 0xFFFFFFFF)
	{
		return 0;
	}

	return 1;
}

float4 PSMain(VertexOut pin) : SV_TARGET0
{
	StructuredBuffer<FMaterial> materialBuffer = GetBuffer<FMaterial>(Constants.MaterialBuffer);
	FMaterial material = materialBuffer[Constants.MaterialID];

	Texture2D texEmissive = GetTexture(material.EmissiveIndex);
	Texture2D texDepth = GetTexture(Constants.bAlphaMask);
	//Texture2D texAO = GetTexture(Constants.AmbientOcclusionIndex);
	const float2 texCoord = pin.TexCoord.xy;

	float4 baseColor = float4(material.BaseColorFactor.rgb, material.BaseColorFactor.a);
	if (IsIndexValid(material.BaseColorIndex))
	{
		Texture2D baseColorTexture = GetTexture(material.BaseColorIndex);
		
		baseColor = baseColorTexture.Sample(AnisotropicSampler, pin.TexCoord);
		baseColor.rgb *= material.BaseColorFactor.rgb;
		
		baseColor.rgb = pow(baseColor.rgb, float3((1.0f / 2.2f).xxx));
	}
	
	float4 emissive = float4(material.EmissiveFactor);
	emissive *= material.EmissiveStrength;
	if (IsIndexValid(material.EmissiveIndex))
	{
		const Texture2D emissiveTexture = GetTexture(material.EmissiveIndex);
		emissive = emissiveTexture.Sample(AnisotropicSampler, pin.TexCoord);
		emissive *= material.EmissiveFactor;
	}
	
	float4 normal = pin.NormalsVS;
	if (IsIndexValid(material.NormalIndex))
	{
		const Texture2D normalTexture = GetTexture(material.NormalIndex);
		float4 normalMap = normalize(normalTexture.Sample(AnisotropicSampler, pin.TexCoord) * 2.0f - 1.0f);
		float4 n = float4(normalize(mul(pin.TBN, normalMap.xyz)), normalMap.w);
		normal = float4(n);
	}
	
	float metalness = material.Metallic;
	float roughness = material.Roughness;
	if (IsIndexValid(material.MetallicRoughnessIndex))
	{
		const Texture2D texMR = GetTexture(material.MetallicRoughnessIndex);
		const float3 metallicRoughnessTexture = texMR.Sample(AnisotropicSampler, texCoord).rgb;
		metalness *= metallicRoughnessTexture.b;
		roughness *= metallicRoughnessTexture.g;
	}
	
	const float depth = texDepth.Sample(AnisotropicSampler, texCoord).r;

	const float3 worldPosition = GetWorldPosition(texCoord, depth, transpose(Scene.InversedViewProjection));
	
	//if (Constants.bSSAO)
	//{
	//	float ao = texAO.Load(uv).r;
	//	baseColor.rgb *= ao;
	//}
	
	const float3 N = normalize(normal.rgb);
	
	
	const float3 V = normalize(Scene.CameraPosition - worldPosition);
	const float NdotV = max(dot(N, V), Epsilon);
	//const float NdotV = max(dot(N, V), 0.0f);

	float3 output = float3(0.0f, 0.0f, 0.0f);
	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	
	// Point lights
	StructuredBuffer<PointLight> PointLights = GetBuffer<PointLight>(Scene.PointLightsBuffer);
	for (uint pointLightIdx = 0; pointLightIdx < Scene.NumPointLights; pointLightIdx++)
	{
		PointLight pointLight = PointLights[pointLightIdx];
		Lo += CalculatePointLight(pointLight, baseColor.rgb, N, V, NdotV, worldPosition, metalness, roughness);
	}
	
	// Spot lights
	StructuredBuffer<SpotLight> SpotLights = GetBuffer<SpotLight>(Scene.SpotLightsBuffer);
	for (uint spotLightIdx = 0; spotLightIdx < Scene.NumSpotLights; spotLightIdx++)
	{
		SpotLight spotLight = SpotLights[spotLightIdx];
		Lo += CalculateSpotLight(spotLight, baseColor.rgb, N, V, NdotV, worldPosition, metalness, roughness);
	}

	// Single directional lighting.
	{
		Lo += CalculateDirectionalLight(Scene.DirectionalPosition, Scene.DirectionalAmbient, Scene.DirectionalIntensity, baseColor.rgb, N, V, NdotV, metalness, roughness);
	}

	output = ((baseColor.rgb * 0.3f)) + Lo;
	output = ((baseColor.rgb)) + Lo;
	output += emissive.rgb;
	return float4(saturate(output), baseColor.a);
}

#endif // TRANSPARENCY_PS_HLSL

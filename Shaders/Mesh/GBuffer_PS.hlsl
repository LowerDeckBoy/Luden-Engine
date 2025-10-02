#ifndef GBUFFER_PS_HLSL
#define GBUFFER_PS_HLSL

#include "GBufferCommon.hlsli"
#include "../Common/Bindless.hlsli"

SamplerState AnisotropicSampler : register(s0);

struct GBuffers
{
	float4 BaseColor			: SV_TARGET0;
	float4 Normal				: SV_TARGET1;
	float4 NormalVS				: SV_TARGET2;
	float4 MetallicRoughness	: SV_TARGET3;
	float4 Emissive				: SV_TARGET4;
	float4 WorldPosition		: SV_TARGET5;
	float4 Depth				: SV_TARGET6;
};

static int IsIndexValid(uint Index)
{
	if (Index == 0xFFFFFFFF)
	{
		return 0;
	}

	return 1;
}

GBuffers PSMain(VertexOut pin) 
{
	GBuffers output = (GBuffers) 0;
	
	const float z = (pin.Position.z / pin.Position.w);
	output.Depth = float4(z, z, z, 1.0f);
	
	StructuredBuffer<FMaterial> materialBuffer = GetBuffer<FMaterial>(Constants.MaterialBuffer);
	FMaterial material = materialBuffer[Constants.MaterialID];
	
	output.WorldPosition = float4(pin.WorldPosition.xyz, z);

	output.Emissive = float4(material.EmissiveFactor.rgb, material.EmissiveFactor.a);
	output.Emissive *= material.EmissiveStrength;
	if (IsIndexValid(material.EmissiveIndex))
	{
		Texture2D emissiveTexture = GetTexture(material.EmissiveIndex);
		output.Emissive = emissiveTexture.Sample(AnisotropicSampler, pin.TexCoord);
		output.Emissive *= material.EmissiveFactor;
	}
	
	output.BaseColor = float4(material.BaseColorFactor.rgb, material.BaseColorFactor.a);
	if (IsIndexValid(material.BaseColorIndex))
	{
		Texture2D baseColorTexture = GetTexture(material.BaseColorIndex);
		
		float4 baseColor = baseColorTexture.Sample(AnisotropicSampler, pin.TexCoord);
		baseColor.rgb *= material.BaseColorFactor.rgb;
		
		if (Constants.bAlphaMask)
		{
			//if (baseColor.a < material.AlphaCutoff)
			if (material.AlphaMode == ALPHA_MODE_MASK && baseColor.a < material.AlphaCutoff)
			{
				discard;
			}
		}
		
		output.BaseColor = float4(baseColor.rgb, 1.0f);
	}
	
	if (Constants.bDrawMeshlets)
	{
		float3 meshletColor = GetMeshletColorHashed(pin.MeshletIndex);
		
		output.BaseColor = float4(meshletColor, 1.0f);
	}
	
	output.Normal = float4(0.0f, 1.0f, 0.0f, 0.0f);
	//output.NormalWS = float4(normalize(pin.NormalsWS.rgb * 0.5f + 0.5f), 1.0f);
	output.NormalVS = float4(normalize(pin.NormalsVS.rgb), 1.0f);
	if (IsIndexValid(material.NormalIndex))
	{
		Texture2D normalTexture = GetTexture(material.NormalIndex);
		float4 normalMap = normalize(2.0f * normalTexture.Sample(AnisotropicSampler, pin.TexCoord) - 1.0f);
		float4 n = float4(normalize(mul(pin.TBN, normalMap.xyz)), normalMap.w);
		output.Normal = float4(n);
	}

	output.MetallicRoughness = float4(0.0f, material.Roughness, material.Metallic, 1.0f);
	if (IsIndexValid(material.MetallicRoughnessIndex))
	{
		Texture2D mrTexture = GetTexture(material.MetallicRoughnessIndex);
		float4 mr = mrTexture.Sample(AnisotropicSampler, pin.TexCoord);
		output.MetallicRoughness = float4(mr.r, mr.g * material.Roughness, mr.b * material.Metallic, 1.0f);
	}

	return output;
}

#endif // GBUFFER_PS_HLSL

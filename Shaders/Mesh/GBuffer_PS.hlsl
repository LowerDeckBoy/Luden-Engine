#ifndef GBUFFER_PS_HLSL
#define GBUFFER_PS_HLSL

#include "GBuffer_RS.hlsli"
#include "GBufferCommon.hlsli"

SamplerState AnisotropicSampler : register(s0);

struct GBuffers
{
	float4 BaseColor;
	float4 Normal;
	float4 MetallicRoughness;
	float4 Emissive;
	float4 WorldPosition;
};

int IsIndexValid(uint Index)
{
	if (Index == 0xFFFFFFFF)
	{
		return 0;
	}

	return 1;
}

Texture2D GetTexture(in uint Index)
{
	Texture2D output = ResourceDescriptorHeap[Index];
	
	return output;
}

[earlydepthstencil]
[RootSignature(GBUFFER_ROOT_SIG)]
GBuffers PSMain(VertexOut pin) : SV_TARGET
{
	GBuffers output = (GBuffers) 0;

	StructuredBuffer<FMaterial> materialBuffer = ResourceDescriptorHeap[Constants.MaterialBuffer];
	FMaterial material = materialBuffer[Constants.MaterialID];
	
	output.WorldPosition = float4(pin.WorldPosition.xyz, 0.0f);
	
	output.Emissive = float4(material.EmissiveFactor);
	if (IsIndexValid(material.EmissiveIndex))
	{
		Texture2D emissiveTexture = ResourceDescriptorHeap[material.EmissiveIndex];
		output.Emissive = emissiveTexture.Sample(AnisotropicSampler, pin.TexCoord);
		output.Emissive *= material.EmissiveFactor;
	}
	
	output.BaseColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (IsIndexValid(material.BaseColorIndex))
	{
		Texture2D baseColorTexture = GetTexture(material.BaseColorIndex);
		
		float4 baseColor = baseColorTexture.Sample(AnisotropicSampler, pin.TexCoord) * float4(material.BaseColorFactor.rgba);
		
		if (Constants.bAlphaMask)
		{
			if (material.AlphaMode == ALPHA_MODE_MASK && baseColor.a < material.AlphaCutoff)
			{
				discard;
			}
		}
		
		output.BaseColor = float4(baseColor.rgb + output.Emissive.rgb, 1.0f);
	}
	
	if (Constants.bDrawMeshlets)
	{
		float3 meshletColor = GetMeshletColorHashed(pin.MeshletIndex);
		
		output.BaseColor = float4(meshletColor, 1.0f);
	}
	
	output.Normal = float4(0.0f, 1.0f, 0.0f, 0.0f);
	if (IsIndexValid(material.NormalIndex))
	{
		Texture2D normalTexture = ResourceDescriptorHeap[material.NormalIndex];
		float4 normalMap = normalize(2.0f * normalTexture.Sample(AnisotropicSampler, pin.TexCoord) - float4(1.0f, 1.0f, 1.0f, 1.0f));
		float4 n = float4(normalize(mul(pin.TBN, normalMap.xyz)), normalMap.w);
		output.Normal = float4(n.rgb, normalMap.w);
	}
	
	// Saving depth into unused Normal's W component.
	const float z = 1.0f - (pin.Position.z / pin.Position.w);
	output.Normal.w = z;

	output.MetallicRoughness = float4(0.0f, material.Roughness, material.Metallic, 1.0f);
	if (IsIndexValid(material.MetallicRoughnessIndex))
	{
		Texture2D mrTexture = ResourceDescriptorHeap[material.MetallicRoughnessIndex];
		float4 mr = mrTexture.Sample(AnisotropicSampler, pin.TexCoord);
		output.MetallicRoughness = float4(mr.r, mr.g * material.Roughness, mr.b * material.Metallic, 1.0f);
	}

	return output;
}

#endif // GBUFFER_PS_HLSL

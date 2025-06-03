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

[RootSignature(GBUFFER_ROOT_SIG)]
GBuffers PSMain(VertexOut pin) : SV_TARGET
{
	GBuffers output = (GBuffers) 0;

	if (IsIndexValid(Material.EmissiveIndex))
	{
		Texture2D emissiveTexture = ResourceDescriptorHeap[Material.EmissiveIndex];
		output.Emissive = emissiveTexture.Sample(AnisotropicSampler, pin.TexCoord);

	}
	
	output.BaseColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (IsIndexValid(Material.BaseColorIndex))
	{
		Texture2D baseColorTexture = GetTexture(Material.BaseColorIndex);
		
		float4 baseColor = baseColorTexture.Sample(AnisotropicSampler, pin.TexCoord) * float4(Material.BaseColorFactor.rgba);
		
		if (Constants.bAlphaMask)
		{
			if (Material.AlphaMode == ALPHA_MODE_MASK && baseColor.a < Material.AlphaCutoff)
			{
				discard;
			}
		}
		
		output.BaseColor = float4(baseColor.rgb, 1.0f);
	}
	
	// Saving depth into unused Normal's W component.
	const float z = 1.0f - (pin.Position.z / pin.Position.w);
	output.Normal.w = z;
	
	if (Constants.bDrawMeshlets)
	{
		float3 meshletColor = GetMeshletColorHashed(pin.MeshletIndex);
		output.BaseColor = float4(meshletColor, 1.0f);
	}
	
	output.Normal = float4(0.0f, 1.0f, 0.0f, 1.0f);
	if (IsIndexValid(Material.NormalIndex))
	{
		Texture2D normalTexture = ResourceDescriptorHeap[Material.NormalIndex];
		float4 normalMap = normalize(2.0f * normalTexture.Sample(AnisotropicSampler, pin.TexCoord) - float4(1.0f, 1.0f, 1.0f, 1.0f));
		float4 n = float4(normalize(mul(pin.TBN, normalMap.xyz)), normalMap.w);
		output.Normal = float4(n.rgb, normalMap.w);
	}
	
	output.MetallicRoughness = float4(0.0f, Material.Roughness, Material.Metallic, 1.0f);
	if (IsIndexValid(Material.MetallicRoughnessIndex))
	{
		Texture2D mrTexture = ResourceDescriptorHeap[Material.MetallicRoughnessIndex];
		float4 mr = mrTexture.Sample(AnisotropicSampler, pin.TexCoord);
		output.MetallicRoughness = float4(mr.r, mr.g * Material.Roughness, mr.b * Material.Metallic, 1.0f);
	}

	return output;
}

#endif // GBUFFER_PS_HLSL

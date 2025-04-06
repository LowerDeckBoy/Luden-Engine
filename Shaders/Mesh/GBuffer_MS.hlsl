#ifndef GBUFFER_MS_HLSL
#define GBUFFER_MS_HLSL

#include "GBuffer_RS.hlsli"
#include "../Mesh.hlsli"
		
#define AS_GROUP_SIZE 32

struct FMaterial
{
	float4 BaseColorFactor;
	float4 EmissiveFactor;

	float AlphaCutoff;
	float Metallic;
	float Roughness;
	float Specular;

	float Transparency;
	float IndexOfRefraction;
	float Anisotropy;
	uint AlphaMode;

	uint BaseColorIndex;
	uint NormalIndex;
	uint MetallicRoughnessIndex;
	uint EmissiveIndex;

};

struct SceneConstants
{
	float4x4 View;
	float4x4 Projection;
};

struct Transform
{
	row_major float4x4 WVP;
	row_major float4x4 World;
};

// Indices to buffers.
struct PushConstants
{
	uint VertexIndex;
	uint MeshletIndex;
	uint MeshletVerticesIndex;
	uint MeshletTrianglesIndex;
	uint MeshletBoundsIndex;
	uint bDrawMeshlets;
	uint bAlphaMask;
};

struct Vertex
{
	float3 Position;
	float2 TexCoord;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
};

struct VertexOut
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : WORLD_POSITION;
	float2 TexCoord : TEXCOORD;
	float3x3 TBN : TBN;
	
	/*
	float3 Normal : NORMAL;
	float4 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	*/
	
	uint MeshletIndex : COLOR0;
};

ConstantBuffer<Transform> Transforms : register(b0);
ConstantBuffer<PushConstants> Constants : register(b1);
ConstantBuffer<FMaterial> Material : register(b2);

VertexOut GetVertexAttributes(Vertex InVertex, uint MeshletIndex)
{
	VertexOut vout;
	
	vout.Position = mul(Transforms.WVP, float4(InVertex.Position, 1.0f));
	vout.WorldPosition = mul(Transforms.World, float4(InVertex.Position, 1.0f));
	vout.TexCoord = InVertex.TexCoord;
	float3 N = normalize(mul((float3x3)Transforms.World, InVertex.Normal));
	float3 T = normalize(mul((float3x3)Transforms.World, InVertex.Tangent));
	float3 B = normalize(mul((float3x3)Transforms.World, InVertex.Bitangent));
	//float3 Bitangent = cross(Normal, Tangent.rgb);
	//Bitangent = normalize(mul((float3x3)Transforms.World, Bitangent));
	
	vout.TBN = float3x3(T, B, N);
	vout.TBN = mul((float3x3)Transforms.World, transpose(vout.TBN));
	//vout.Bitangent = normalize(mul(Transforms.World, InVertex.Tangent));
	vout.MeshletIndex = MeshletIndex;
	
	return vout;
}

struct Payload
{
	uint MeshletIndices[AS_GROUP_SIZE];
};

groupshared Payload sPayload;

[NumThreads(AS_GROUP_SIZE, 1, 1)]
void ASMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint DispatchThreadID : SV_DispatchThreadID,
	uint GroupID : SV_GroupID)
{
	sPayload.MeshletIndices[GroupThreadID] = DispatchThreadID;
	
	DispatchMesh(AS_GROUP_SIZE, 1, 1, sPayload);
}

[RootSignature(GBUFFER_ROOT_SIG)]
[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint GroupID : SV_GroupID,
	in payload Payload payload,
	out indices uint3 Triangles[MAX_TRIANGLES],
	out vertices VertexOut Vertices[MAX_VERTICES])
{
	StructuredBuffer<Meshlet> Meshlets = ResourceDescriptorHeap[Constants.MeshletIndex];
	StructuredBuffer<Vertex> VertexBuffer = ResourceDescriptorHeap[Constants.VertexIndex];
	StructuredBuffer<uint> MeshletVertices = ResourceDescriptorHeap[Constants.MeshletVerticesIndex];
	StructuredBuffer<uint> MeshletTriangles = ResourceDescriptorHeap[Constants.MeshletTrianglesIndex];
	
	uint meshletIndex = payload.MeshletIndices[GroupID];
	Meshlet meshlet = Meshlets[meshletIndex];

	SetMeshOutputCounts(meshlet.VertexCount, meshlet.TriangleCount);

	if (GroupThreadID < meshlet.TriangleCount)
	{
		uint packed = MeshletTriangles.Load(meshlet.TriangleOffset + GroupThreadID);

		Triangles[GroupThreadID] = UnpackTriangle(packed);
	}
	
	if (GroupThreadID < meshlet.VertexCount)
	{
		uint vertexIndex = MeshletVertices[meshlet.VertexOffset + GroupThreadID];
		Vertex vertex = VertexBuffer[vertexIndex];
		
		Vertices[GroupThreadID] = GetVertexAttributes(vertex, GroupID);
	}

}

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

	if (IsIndexValid(Material.BaseColorIndex))
	{
		Texture2D baseColorTexture = GetTexture(Material.BaseColorIndex);
		
		float4 baseColor = baseColorTexture.Sample(AnisotropicSampler, pin.TexCoord);
	
		if (Constants.bAlphaMask)
		{
			if (Material.AlphaMode == 2 && baseColor.a < Material.AlphaCutoff)
			{
				discard;
			}
		
			//if (Material.Transparency > 0.0f && baseColor.a < Material.AlphaCutoff)
			//{
			//	
			//}
		}
		
		output.BaseColor = baseColor;
	}
	else
	{
		output.BaseColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	}
	
	//if (Constants.bDrawMeshlets)
	//{
	//	float3 meshletColor = GetMeshletColorHashed(pin.MeshletIndex);
	//	//return float4(meshletColor, 1.0f);
	//}
	
	if (IsIndexValid(Material.NormalIndex))
	{
		Texture2D normalTexture = ResourceDescriptorHeap[Material.NormalIndex];
		float4 normalMap = normalize(2.0f * normalTexture.Sample(AnisotropicSampler, pin.TexCoord) - float4(1.0f, 1.0f, 1.0f, 1.0f));
		float4 n = float4(normalize(mul(pin.TBN, normalMap.xyz)), normalMap.w);
		output.Normal = float4(n.rgb, normalMap.w);
	}
	
	if (IsIndexValid(Material.MetallicRoughnessIndex))
	{
		Texture2D mrTexture = ResourceDescriptorHeap[Material.MetallicRoughnessIndex];
		float4 mr = mrTexture.Sample(AnisotropicSampler, pin.TexCoord);
		output.MetallicRoughness = float4(0.0f, mr.g * Material.Roughness, mr.b * Material.Metallic, 1.0f);
	}
	else
	{
		output.MetallicRoughness = float4(0.0f, Material.Roughness, Material.Metallic, 1.0f);
	}
	
	if (IsIndexValid(Material.EmissiveIndex))
	{
		Texture2D emissiveTexture = ResourceDescriptorHeap[Material.EmissiveIndex];
		output.Emissive = emissiveTexture.Sample(AnisotropicSampler, pin.TexCoord);

	}
	
	// Saving depth into unused Normal's W component.
	//const float z = 1.0f - (pin.Position.z / pin.Position.w);
	//output.Normal.w = z;
	
	
	return output;

}

#endif // GBUFFER_MS_HLSL
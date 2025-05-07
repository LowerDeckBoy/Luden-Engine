#ifndef MESH_HLSL
#define MESH_HLSL

#include "Mesh.hlsli"

#define ROOT_SIG "RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |"\
	"DENY_HULL_SHADER_ROOT_ACCESS |"\
	"DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
	"DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
	"CBV(b0, space=0), "\
	"RootConstants(num32BitConstants=7, b1), "\
	"RootConstants(num32BitConstants=20, b2), "\
	"StaticSampler(s0, "\
		"addressU = TEXTURE_ADDRESS_WRAP, "\
		"addressV = TEXTURE_ADDRESS_WRAP, "\
		"filter = FILTER_MAXIMUM_ANISOTROPIC )"

		
#define AS_GROUP_SIZE 32

struct FMaterial
{
	float4 BaseColorFactor;
	float4 EmissiveFactor;

	float AlphaCutoff;
	float Metallic;
	float Roughness;
	float Specular;

	float Glossiness;
	float Transparency ;
	float IndexOfRefraction;
	float Anisotropy;

	uint BaseColorIndex;
	uint NormalIndex;
	uint MetallicRoughnessIndex;
	uint EmissiveIndex;

};

struct Frustum
{
	float4 Planes[6];
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
	float4 Tangent;
};

struct VertexOut
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : WORLD_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 Tangent : TANGENT;
	uint MeshletIndex : COLOR0;
};

ConstantBuffer<Transform> Transforms : register(b0);
ConstantBuffer<PushConstants> Constants : register(b1);
ConstantBuffer<FMaterial> Material : register(b2);


bool IsMeshletVisible(MeshletBound Bound)
{

	return true;
}



VertexOut GetVertexAttributes(Vertex InVertex, uint MeshletIndex)
{
	VertexOut vout;
	
	vout.Position = mul(Transforms.WVP, float4(InVertex.Position, 1.0f));
	vout.WorldPosition = mul(Transforms.World, float4(InVertex.Position, 1.0f));
	vout.TexCoord = InVertex.TexCoord;
	vout.Normal = normalize(mul((float3x3) Transforms.World, InVertex.Normal));
	vout.Tangent = normalize(mul(Transforms.World, InVertex.Tangent));
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

[RootSignature(ROOT_SIG)]
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

[RootSignature(ROOT_SIG)]
float4 PSMain(VertexOut pin) : SV_TARGET
{
	float3 output = pin.Normal;
	
	Texture2D baseColorTexture = ResourceDescriptorHeap[Material.BaseColorIndex];
	float4 baseColor = baseColorTexture.Sample(AnisotropicSampler, pin.TexCoord);

	if (Constants.bAlphaMask)
	{
		if (baseColor.a < Material.AlphaCutoff)
		{
			discard;
		}
	}

	if (Constants.bDrawMeshlets)
	{
		float3 meshletColor = GetMeshletColorHashed(pin.MeshletIndex);
		return float4(meshletColor, 1.0f);
	}
	
	return float4(baseColor.rgb, 1.0f);
	
	float3 N = normalize(pin.Normal);
	float4 T = normalize(pin.Tangent);
	//float3 bitangent = -normalize(T.xyz - dot(T.xyz, N) * N) * T.w;
	float3 bitangent = normalize(cross(T.xyz, N)) * T.w;
	float3x3 TBN = float3x3(T.xyz, bitangent, N);
	TBN = mul((float3x3)Transforms.World, transpose(TBN));
	
	Texture2D normalTexture = ResourceDescriptorHeap[Material.NormalIndex];
	float4 normalMap = normalize(2.0f * normalTexture.Sample(AnisotropicSampler, pin.TexCoord) - float4(1.0f, 1.0f, 1.0f, 1.0f));
	float4 n = float4(normalize(mul(TBN, normalMap.xyz)), normalMap.w);	
	return float4(n.rgb, 1.0f);
	
	
	//
	//return float4(output, 1.0f);
}

#endif // MESH_HLSL

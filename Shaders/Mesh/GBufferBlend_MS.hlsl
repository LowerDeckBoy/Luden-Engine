#ifndef GBUFFER_MS_HLSL
#define GBUFFER_MS_HLSL

#include "../Material.hlsli"
#include "../Mesh.hlsli"
#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"
		
#define AS_GROUP_SIZE 32

#define GBUFFER_ROOT_SIG "RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |"\
	"DENY_HULL_SHADER_ROOT_ACCESS |"\
	"DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
	"DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
	"RootConstants(num32BitConstants=14, b1), "\
	"RootConstants(num32BitConstants=28, b2), "\
	"StaticSampler(s0, "\
		"addressU = TEXTURE_ADDRESS_WRAP, "\
		"addressV = TEXTURE_ADDRESS_WRAP, "\
		"filter = FILTER_MIN_MAG_MIP_LINEAR )"

struct Payload
{
	uint MeshletIndices[AS_GROUP_SIZE];
};
		
struct Transform
{
	float4x4 WVP;
	float4x4 World;
	float4x4 PreviousWorld;
};

struct CameraConsts
{
	float3 Position;
	uint pad;
	float4 Planes[6];
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
	uint bMeshletCulling;
	uint bAlphaMask;
	uint TransformBuffer;
	uint MaterialBuffer;
	uint MaterialID;
	uint TransformID;
	float NearZ;
	float FarZ;
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
	float4 CurrPosition : CURR_POSITION;
	float4 PrevPosition : PREV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3x3 TBN : TBN;
	float4 NormalsVS : NORMAL_VS;
	uint MeshletIndex : COLOR0;
};
groupshared Payload sPayload;
ConstantBuffer<PushConstants> Constants : register(b1);
ConstantBuffer<CameraConsts> CameraConstants : register(b2);

VertexOut GetVertexAttributes(Vertex InVertex, uint MeshletIndex)
{
	VertexOut vout;
	
	StructuredBuffer<Transform> transformBuffer = ResourceDescriptorHeap[Constants.TransformBuffer];
	Transform transform = transformBuffer[Constants.TransformID];
	
	const float4x4 world = transform.World;
	vout.Position		= mul(transform.WVP, float4(InVertex.Position, 1.0f));
	vout.CurrPosition	= mul(transform.WVP, float4(InVertex.Position, 1.0f));
	vout.PrevPosition	= mul(transform.PreviousWorld, float4(InVertex.Position, 1.0f));
	vout.WorldPosition	= mul(world, float4(InVertex.Position, 1.0f));

	vout.TexCoord = InVertex.TexCoord;

	float3 N = normalize(mul((float3x3)world, InVertex.Normal));
	float3 T = normalize(mul((float3x3)world, InVertex.Tangent));
	float3 B = normalize(mul((float3x3)world, InVertex.Bitangent));
	
	vout.TBN = mul((float3x3)world, transpose(float3x3(T, B, N)));
	vout.NormalsVS = float4(N, 1.0f);
	
	vout.MeshletIndex = MeshletIndex;
	
	return vout;
}

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
struct GBuffers
{
	float4 BaseColor : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 NormalVS : SV_TARGET2;
	float4 MotionVectors : SV_TARGET3;
	float4 MetallicRoughness : SV_TARGET4;
	float4 Emissive : SV_TARGET5;
	float4 WorldPosition : SV_TARGET6;
	float4 Depth : SV_TARGET7;
};

int IsIndexValid(uint Index)
{
	if (Index == 0xFFFFFFFF)
	{
		return 0;
	}

	return 1;
}

SamplerState AnisotropicSampler : register(s0);

GBuffers PSMain(VertexOut pin)
{
	GBuffers output = (GBuffers) 0;

	StructuredBuffer<FMaterial> materialBuffer = GetBuffer<FMaterial>(Constants.MaterialBuffer);
	FMaterial material = materialBuffer[Constants.MaterialID];
	
	output.WorldPosition = float4(pin.WorldPosition.xyz, 1.0f);

	output.Emissive = float4(material.EmissiveFactor.rgb, material.EmissiveFactor.a);
	output.Emissive *= material.EmissiveStrength;
	if (IsIndexValid(material.EmissiveIndex))
	{
		Texture2D emissiveTexture = GetTexture(material.EmissiveIndex);
		output.Emissive = emissiveTexture.Sample(AnisotropicSampler, pin.TexCoord);
		output.Emissive *= material.EmissiveFactor;
	}
	
	output.BaseColor = float4(material.BaseColorFactor.rgb, 1.0f);
	if (IsIndexValid(material.BaseColorIndex))
	{
		Texture2D baseColorTexture = GetTexture(material.BaseColorIndex);
		
		float4 baseColor = baseColorTexture.Sample(AnisotropicSampler, pin.TexCoord);
		baseColor.rgb *= material.BaseColorFactor.rgb;
		
		output.BaseColor = float4(baseColor.rgb, baseColor.a);
	}
	
	if (Constants.bDrawMeshlets)
	{
		float3 meshletColor = GetMeshletColorHashed(pin.MeshletIndex);
		
		output.BaseColor = float4(meshletColor, 1.0f);
	}
	
	output.Normal = float4(0.0f, 1.0f, 0.0f, 0.0f);
	output.NormalVS = float4(pin.NormalsVS.rgb, 1.0f);
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

#endif // GBUFFER_MS_HLSL

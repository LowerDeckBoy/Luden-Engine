#ifndef MESH_HLSL
#define MESH_HLSL

#include "Mesh.hlsli"

static const uint MAX_TRIANGLES = 126;
static const uint MAX_VERTICES = 64;

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
	uint bDrawMeshlets;
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
	float4	Position		: SV_POSITION;
	float4	WorldPosition	: WORLD_POSITION;
	float2	TexCoord		: TEXCOORD;
	float3	Normal			: NORMAL;
	float4	Tangent			: TANGENT;
	uint	MeshletIndex	: COLOR0;
};

struct VertexData
{
	uint Index;
};

ConstantBuffer<Transform>		Transforms			: register(b0);
ConstantBuffer<PushConstants>	Constants			: register(b1);
StructuredBuffer<uint>			UniqueVertexIndices : register(t0);
StructuredBuffer<uint>			PrimitiveIndices	: register(t1);

uint Hash(uint Value)
{
	Value = (Value + 0x7ed55d16) + (Value << 12);
	Value = (Value ^ 0xc761c23c) ^ (Value >> 19);
	Value = (Value + 0x165667b1) + (Value << 5);
	Value = (Value + 0xd3a2646c) ^ (Value << 9);
	Value = (Value + 0xfd7046c5) + (Value << 3);
	Value = (Value ^ 0xb55a4f09) ^ (Value >> 16);
	return Value;
}

// Get Triangle from given location and unpack it.
uint3 GetPrimitive(uint Location)
{
	uint packed = PrimitiveIndices[Location];
	
	uint idx0 = (packed		 ) & 0x3FF;
	uint idx1 = (packed >> 10) & 0x3FF;
	uint idx2 = (packed >> 20) & 0x3FF;
	
	return uint3(idx0, idx1, idx2);
}

float3 GetMeshletColor(uint MeshletIndex)
{
	uint mhash = Hash(MeshletIndex);
	
	return float3(
		float(mhash & 255),
		float((mhash >> 8) & 255),
		float((mhash >> 16) & 255)) / 255.0;

	//return float3(float(MeshletIndex & 1),
	//		float(MeshletIndex & 3) / 4,
	//		float(MeshletIndex & 7) / 8);
}

VertexOut GetVertexAttributes(uint VertexIndex, uint MeshletIndex)
{
	StructuredBuffer<Vertex> VertexBuffer = ResourceDescriptorHeap[Constants.VertexIndex];
	Vertex vertex = VertexBuffer[VertexIndex];

	VertexOut vout;
	
	vout.Position		= mul(Transforms.WVP, float4(vertex.Position, 1.0f));
	vout.TexCoord		= vertex.TexCoord;
	vout.Normal			= mul((float3x3)Transforms.World, vertex.Normal);
	vout.Tangent		= mul(Transforms.World, vertex.Tangent);
	vout.MeshletIndex	= MeshletIndex;
	
	return vout;
}

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint GroupID : SV_GroupID,
	out indices uint3 Triangles[MAX_TRIANGLES],
	out vertices VertexOut Vertices[MAX_VERTICES])
{
	StructuredBuffer<Meshlet> Meshlets = ResourceDescriptorHeap[Constants.MeshletIndex];
	
	Meshlet meshlet = Meshlets[GroupID];

	SetMeshOutputCounts(meshlet.VertexCount, meshlet.TriangleCount);

	if (GroupThreadID < meshlet.TriangleCount)
	{
		Triangles[GroupThreadID] = GetPrimitive(meshlet.TriangleOffset + GroupThreadID);
	}
	
	if (GroupThreadID < meshlet.VertexCount)
	{
		uint vertexIndex = UniqueVertexIndices[meshlet.VertexOffset + GroupThreadID];
		Vertices[GroupThreadID] = GetVertexAttributes(vertexIndex, GroupID);
	}
}

float4 PSMain(VertexOut pin) : SV_TARGET
{
	float3 output = pin.Normal;
	
	if (Constants.bDrawMeshlets)
	{
		float3 meshletColor = GetMeshletColor(pin.MeshletIndex);
		return float4(meshletColor, 1.0f);
	}

	return float4(output, 1.0f);
}

#endif // MESH_HLSL

#ifndef MESH_HLSL
#define MESH_HLSL

#include "Mesh.hlsli"

static const uint MAX_TRIANGLES = 126;
static const uint MAX_VERTICES = 64;

struct cbPerObject
{
	row_major float4x4 WVP;
	row_major float4x4 World;
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
	float3	Color			: COL;
	uint	MeshletIndex	: COLOR0;
};

struct VertexData
{
	uint Index;
};

ConstantBuffer<cbPerObject> PerObject : register(b0);
//ConstantBuffer<VertexData>	VertexBuffer		: register(b1);
StructuredBuffer<Vertex> VertexBuffer : register(t0);
StructuredBuffer<Meshlet> Meshlets : register(t1);
//ByteAddressBuffer			UniqueVertexIndices	: register(t2);
StructuredBuffer<uint> UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);

uint3 UnpackPrimitive(uint primitive)
{
	return uint3(
		(primitive) & 0x3FF,
		(primitive >> 10) & 0x3FF,
		(primitive >> 20) & 0x3FF);
}

uint3 GetPrimitive(Meshlet meshlet, uint index)
{
	return UnpackPrimitive(PrimitiveIndices[meshlet.TriangleOffset + index]);
}

VertexOut GetVertexAttributes(uint VertexIndex)
{
	VertexOut vout = (VertexOut) 0;
	
	return vout;
}

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint GroupID : SV_GroupID,
	out indices uint3 Triangles[126],
	out vertices VertexOut Vertices[64])
{
	Meshlet meshlet = Meshlets[GroupID];

	SetMeshOutputCounts(meshlet.VertexCount, meshlet.TriangleCount);

	if (GroupThreadID < meshlet.TriangleCount)
	{
		Triangles[GroupThreadID] = GetPrimitive(meshlet, GroupThreadID);
	}

	if (GroupThreadID < meshlet.VertexCount)
	{
		uint vertexIndex = UniqueVertexIndices[meshlet.VertexOffset + GroupThreadID].r;
		
		Vertices[GroupThreadID].Position = mul(PerObject.WVP, float4(VertexBuffer[vertexIndex].Position, 1.0f));
		Vertices[GroupThreadID].Normal = mul((float3x3) PerObject.World, VertexBuffer[vertexIndex].Normal);
		
		Vertices[GroupThreadID].Color = float3(
			float(GroupID & 1),
			float(GroupID & 3) / 4,
			float(GroupID & 7) / 8);
		
	}
	
	//for (uint i = GroupThreadID; i < meshlet.TriangleCount; i += 128)
	//{
	//	uint offset = meshlet.TriangleOffset + i * 3;
	//	Triangles[i] = uint3(
	//        PrimitiveIndices[offset],
	//        PrimitiveIndices[offset + 1],
	//        PrimitiveIndices[offset + 2]
	//    );
	//}
	//for (uint j = GroupThreadID; j < meshlet.VertexCount; j += 128)
	//{
	//	uint index = UniqueVertexIndices[meshlet.VertexOffset + j];
	//	Vertices[j] = GetVertexAttributes(index, GroupID);
	//}

}

float4 PSMain(VertexOut pin) : SV_TARGET
{
	return float4(pin.Color, 1.0f);
}

#endif // MESH_HLSL

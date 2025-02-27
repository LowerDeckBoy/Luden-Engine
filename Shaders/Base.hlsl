#ifndef BASE_HLSL
#define BASE_HLSL

#include "Mesh.hlsli"
#include "Camera.hlsli"

struct Indices
{
	uint PositionsIndex;
	uint TexCoordsIndex;
	uint NormalsIndex;
	uint TangentsIndex;
};

struct MeshInfo
{
	//uint		Index;
	float4x4	WVP;
	float4x4	World;
	
	Indices Buffers;
	
	uint		PositionsIndex;
	uint		TexCoordsIndex;
	uint		NormalsIndex;
	uint		TangentsIndex;
};
StructuredBuffer<MeshInfo> Info;
ConstantBuffer<MeshInfo> cbMeshInfo;

struct Vertex
{
	float3 Position;
	float2 UV;
	float3 Normal;
	float4 Tangent;
};


//Vertex LoadVertex(MeshInfo Mesh, uint VertexId)
Vertex LoadVertex(Indices Mesh, uint VertexId)
{
	StructuredBuffer<float3> positions	= ResourceDescriptorHeap[Mesh.PositionsIndex];
	StructuredBuffer<float2> texCoords	= ResourceDescriptorHeap[Mesh.TexCoordsIndex];
	StructuredBuffer<float3> normals	= ResourceDescriptorHeap[Mesh.NormalsIndex];
	StructuredBuffer<float4> tangents	= ResourceDescriptorHeap[Mesh.TangentsIndex];
	
	Vertex output;
	output.Position = positions.Load(VertexId).xyz;
	output.UV		= texCoords.Load(VertexId).xy;
	output.Normal	= normals.Load(VertexId).xyz;
	output.Tangent	= tangents.Load(VertexId).xyzw;
	
	return output;
}

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : WORLD_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
};


// VS
VSOutput VSMain(uint VertexID : SV_VertexID)
{
	//MeshInfo meshInfo = Info.Load()

	Vertex vertex = LoadVertex(cbMeshInfo.Buffers, VertexID);
	
	float2 vertices[3] =
	{
		float2(-0.5f, -0.5f),
		float2(+0.5f, -0.5f),
		float2(+0.0f, +0.5f)
	};
	
	float3 colors[3] =
	{
		float3(1.0f, 0.0f, 0.0f),
		float3(0.0f, 1.0f, 0.0f),
		float3(0.0f, 0.0f, 1.0f),
	};
	
	VSOutput output;
	//output.Position			= mul(cbMeshInfo.WVP, float4(vertex.Position, 1.0f));
	output.Position			= float4(vertices[VertexID], 0.0f, 1.0f);
	output.WorldPosition	= mul(cbMeshInfo.World, float4(vertex.Position, 1.0f));
	output.TexCoord			= vertex.UV;
	output.Normal			= colors[VertexID];
	//output.Normal			= normalize(mul((float3x3) cbMeshInfo.World, vertex.Normal));
	output.Tangent			= normalize(mul((float3x3) cbMeshInfo.World, vertex.Tangent.xyz));
	
	float3 bitangent = cross(output.Normal, output.Tangent);
	output.Bitangent = bitangent;
	
	return output;
}

// PS

float4 PSMain(VSOutput pin) : SV_TARGET0
{
	return float4(pin.Normal, 1.0f);
}

#endif // BASE_HLSL

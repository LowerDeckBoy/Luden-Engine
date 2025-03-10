#ifndef MESH_HLSLI
#define MESH_HLSLI


struct StaticMeshData
{
	uint PositionsIndex;
	uint TexCoordsIndex;
	uint NormalsIndex;
	uint TangentsIndex;
};



struct Meshlet
{
	uint VertexOffset;
	uint TriangleOffset;
	uint VertexCount;
	uint TriangleCount;
};

//struct Vertex
//{
//	float3 Position;
//	float2 TexCoords;
//	float3 Normal;
//	float4 Tangent;
//};

#endif // MESH_HLSLI

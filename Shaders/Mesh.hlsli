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
	uint VertexCount;
	uint VertexOffset;
	uint TriangleCount;
	uint TriangleOffset;
};

//struct Vertex
//{
//	float3 Position;
//	float2 TexCoords;
//	float3 Normal;
//	float3 Tangent;
//};

#endif // MESH_HLSLI

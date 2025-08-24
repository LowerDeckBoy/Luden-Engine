#ifndef GBUFFER_MS_HLSL
#define GBUFFER_MS_HLSL

#include "GBuffer_RS.hlsli"
#include "GBufferCommon.hlsli"

static StructuredBuffer<Meshlet>	Meshlets			= GetBuffer<Meshlet>(Constants.MeshletIndex);
static StructuredBuffer<Vertex>		VertexBuffer		= GetBuffer<Vertex>(Constants.VertexIndex);
static StructuredBuffer<uint>		MeshletVertices		= GetBuffer<uint>(Constants.MeshletVerticesIndex);
static StructuredBuffer<uint>		MeshletTriangles	= GetBuffer<uint>(Constants.MeshletTrianglesIndex);

[RootSignature(GBUFFER_RS)]
[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
	uint 	GroupThreadID : SV_GroupThreadID,
	uint 	GroupID : SV_GroupID,
	in 		payload Payload payload,
	out 	indices uint3 Triangles[MAX_TRIANGLES],
	out 	vertices VertexOut Vertices[MAX_VERTICES])
{
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

#endif // GBUFFER_MS_HLSL

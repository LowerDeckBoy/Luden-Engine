#ifndef GBUFFER_MS_HLSL
#define GBUFFER_MS_HLSL

#include "GBuffer_RS.hlsli"
#include "GBufferCommon.hlsli"

static StructuredBuffer<Meshlet>	Meshlets			= ResourceDescriptorHeap[Constants.MeshletIndex];
static StructuredBuffer<Vertex>		VertexBuffer		= ResourceDescriptorHeap[Constants.VertexIndex];
static StructuredBuffer<uint>		MeshletVertices		= ResourceDescriptorHeap[Constants.MeshletVerticesIndex];
static StructuredBuffer<uint>		MeshletTriangles	= ResourceDescriptorHeap[Constants.MeshletTrianglesIndex];

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

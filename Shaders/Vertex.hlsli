#ifndef VERTEX_HLSLI
#define VERTEX_HLSLI

struct Vertex
{
	float3 Position;
	float2 TexCoord;
	float3 Normal;
	float4 Tangent;
};

struct VertexOutput
{
	float4	Position;
	float4	WorldPosition;
	float2	TexCoord;
	float3	Normal;
	float4	Tangent;
	uint	MeshletIndex;
};

VertexOutput GetVertexAttributes(Vertex InVertex, uint MeshletIndex)
{
	VertexOutput vout;

	return vout;
}

#endif // VERTEX_HLSLI

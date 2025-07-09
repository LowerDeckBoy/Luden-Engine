#ifndef GBUFFER_COMMON_HLSLI
#define GBUFFER_COMMON_HLSLI

#include "../Material.hlsli"
#include "../Mesh.hlsli"

#define AS_GROUP_SIZE 32

struct Payload
{
	uint MeshletIndices[AS_GROUP_SIZE];
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
	float2 TexCoord : TEXCOORD;
	float3x3 TBN : TBN;
	uint MeshletIndex : COLOR0;
};

ConstantBuffer<PushConstants>	Constants		: register(b1);
ConstantBuffer<CameraConsts>	CameraConstants : register(b2);

VertexOut GetVertexAttributes(Vertex InVertex, uint MeshletIndex)
{
	VertexOut vout;
	
	StructuredBuffer<Transform> transformBuffer = ResourceDescriptorHeap[Constants.TransformBuffer];
	Transform transform = transformBuffer[Constants.TransformID];
	
	vout.Position		= mul(transform.WVP,   float4(InVertex.Position, 1.0f));
	vout.WorldPosition  = mul(transform.World, float4(InVertex.Position, 1.0f));

	vout.TexCoord		= InVertex.TexCoord;
	
	float3 N = normalize(mul((float3x3)transform.World, InVertex.Normal));
	float3 T = normalize(mul((float3x3)transform.World, InVertex.Tangent));
	float3 B = normalize(mul((float3x3)transform.World, InVertex.Bitangent));

	vout.TBN = float3x3(T, B, N);
	vout.TBN = mul((float3x3) transform.World, transpose(vout.TBN));

	vout.MeshletIndex = MeshletIndex;
	
	return vout;
}

#endif // #define GBUFFER_COMMON_HLSLI

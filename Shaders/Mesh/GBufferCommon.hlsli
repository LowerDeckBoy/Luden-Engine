#ifndef GBUFFER_COMMON_HLSLI
#define GBUFFER_COMMON_HLSLI

#include "GBuffer_RS.hlsli"
#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"
#include "../Material.hlsli"
#include "../Mesh.hlsli"

#define AS_GROUP_SIZE 32

struct Payload
{
	uint MeshletIndices[AS_GROUP_SIZE];
};

struct Transform
{
	float4x4 World;
	float4x4 WorldView;
	float4x4 WorldViewProjection;
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
	float4		Position		: SV_POSITION;
	float4		WorldPosition	: WORLD_POSITION;
	float4		CurrPosition	: CURR_POSITION;
	float4		PrevPosition	: PREV_POSITION;
	float2		TexCoord		: TEXCOORD;
	float3x3	TBN				: TBN;
	float4		NormalsVS		: NORMAL_VS;
	uint		MeshletIndex	: COLOR0;
};

ConstantBuffer<PushConstants>	Constants		: register(b1);
ConstantBuffer<CameraConsts>	CameraConstants : register(b2);

VertexOut GetVertexAttributes(Vertex InVertex, uint MeshletIndex)
{
	VertexOut vout;
	
	StructuredBuffer<Transform> transformBuffer = ResourceDescriptorHeap[Constants.TransformBuffer];
	Transform transform = transformBuffer[Constants.TransformID];
	
	float4x4 world = transform.World;
	vout.Position		= mul(float4(InVertex.Position, 1.0f), transpose(transform.WorldViewProjection));
	vout.CurrPosition	= mul(float4(InVertex.Position, 1.0f), transpose(transform.WorldViewProjection));
	vout.PrevPosition   = mul(float4(InVertex.Position, 1.0f), transpose(transform.PreviousWorld));
	vout.WorldPosition	= mul(float4(InVertex.Position, 1.0f), world);

	vout.TexCoord = InVertex.TexCoord;

	float3 N = normalize(mul(InVertex.Normal,	 (float3x3)world));
	float3 T = normalize(mul(InVertex.Tangent,	 (float3x3)world));
	float3 B = normalize(mul(InVertex.Bitangent, (float3x3)world));
	
	vout.TBN = mul((float3x3)world, transpose(float3x3(T, B, N)));
	vout.NormalsVS = mul(float4(InVertex.Normal, 0.0f), (transform.WorldView));
	
	vout.MeshletIndex = MeshletIndex;
	
	return vout;
}

#endif // #define GBUFFER_COMMON_HLSLI

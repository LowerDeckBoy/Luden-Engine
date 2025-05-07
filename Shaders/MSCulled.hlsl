#ifndef MESH_HLSL
#define MESH_HLSL

#include "Mesh.hlsli"

#define ROOT_SIG "RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |"\
	"DENY_HULL_SHADER_ROOT_ACCESS |"\
	"DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
	"DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
	"CBV(b0, space=0), "\
	"RootConstants(num32BitConstants=6, b1), "\
	"StaticSampler(s0, "\
		"addressU = TEXTURE_ADDRESS_WRAP, "\
		"filter = FILTER_MAXIMUM_ANISOTROPIC )"

		
#define AS_GROUP_SIZE 32

struct Frustum
{
	float4 Planes[6];
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

// Indices to buffers.
struct PushConstants
{
	uint VertexIndex;
	uint MeshletIndex;
	uint MeshletVerticesIndex;
	uint MeshletTrianglesIndex;
	uint MeshletBoundsIndex;
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
	float4 Position			: SV_POSITION;
	float4 WorldPosition	: WORLD_POSITION;
	float2 TexCoord			: TEXCOORD;
	float3 Normal			: NORMAL;
	float4 Tangent			: TANGENT;
	uint MeshletIndex		: COLOR0;
};

ConstantBuffer<Transform> Transforms : register(b0);
ConstantBuffer<PushConstants> Constants : register(b1);

float SignedPointPlaneDistance(float3 P, float3 planeN, float3 planeP)
{
	float d = dot(normalize(planeN), P - planeP);
	return d;
};

//bool VisibleFrustumPlanes(float4 sphere)
//{
//	float d0 = SignedPointPlaneDistance(sphere.xyz, Scene.Frustum.Planes[0].Normal, Scene.Frustum.Planes[0].Position);
//	float d1 = SignedPointPlaneDistance(sphere.xyz, Scene.Frustum.Planes[1].Normal, Scene.Frustum.Planes[1].Position);
//	float d2 = SignedPointPlaneDistance(sphere.xyz, Scene.Frustum.Planes[2].Normal, Scene.Frustum.Planes[2].Position);
//	float d3 = SignedPointPlaneDistance(sphere.xyz, Scene.Frustum.Planes[3].Normal, Scene.Frustum.Planes[3].Position);
//	float d4 = SignedPointPlaneDistance(sphere.xyz, Scene.Frustum.Planes[4].Normal, Scene.Frustum.Planes[4].Position);
//	float d5 = SignedPointPlaneDistance(sphere.xyz, Scene.Frustum.Planes[5].Normal, Scene.Frustum.Planes[5].Position);
//
//    // Determine if we're on the positive half space of frustum planes
//	bool pos0 = (d0 >= 0);
//	bool pos1 = (d1 >= 0);
//	bool pos2 = (d2 >= 0);
//	bool pos3 = (d3 >= 0);
//	bool pos4 = (d4 >= 0);
//	bool pos5 = (d5 >= 0);
//
//	bool inside = pos0 && pos1 && pos2 && pos3 && pos4 && pos5;
//	return inside;
//}

bool IsMeshletVisible(MeshletBound Bound)
{

	return true;
}



VertexOut GetVertexAttributes(Vertex InVertex, uint MeshletIndex)
{
	VertexOut vout;
	
	vout.Position		= mul(Transforms.WVP, float4(InVertex.Position, 1.0f));
	vout.WorldPosition	= mul(Transforms.World, float4(InVertex.Position, 1.0f));
	vout.TexCoord		= InVertex.TexCoord;
	vout.Normal			= normalize(mul((float3x3) Transforms.World, InVertex.Normal));
	vout.Tangent		= mul(Transforms.World, InVertex.Tangent);
	vout.MeshletIndex	= MeshletIndex;
	
	return vout;
}

struct Payload
{
	uint MeshletIndices[AS_GROUP_SIZE];
};

groupshared Payload sPayload;

[NumThreads(AS_GROUP_SIZE, 1, 1)]
void ASMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint DispatchThreadID : SV_DispatchThreadID,
	uint GroupID : SV_GroupID)
{
	sPayload.MeshletIndices[GroupThreadID] = DispatchThreadID;
	
	DispatchMesh(AS_GROUP_SIZE, 1, 1, sPayload);
}

[RootSignature(ROOT_SIG)]
[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint GroupID : SV_GroupID,
	in payload Payload payload,
	out indices uint3 Triangles[MAX_TRIANGLES],
	out vertices VertexOut Vertices[MAX_VERTICES])
{
	StructuredBuffer<Meshlet> Meshlets = ResourceDescriptorHeap[Constants.MeshletIndex];
	StructuredBuffer<Vertex> VertexBuffer = ResourceDescriptorHeap[Constants.VertexIndex];
	StructuredBuffer<uint> MeshletVertices = ResourceDescriptorHeap[Constants.MeshletVerticesIndex];
	StructuredBuffer<uint> MeshletTriangles = ResourceDescriptorHeap[Constants.MeshletTrianglesIndex];
	
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

float4 PSMain(VertexOut pin) : SV_TARGET
{
	float3 output = pin.Normal;
	
	if (Constants.bDrawMeshlets)
	{
		float3 meshletColor = GetMeshletColorHashed(pin.MeshletIndex);
		return float4(meshletColor, 1.0f);
	}

	return float4(output, 1.0f);
}

#endif // MESH_HLSL

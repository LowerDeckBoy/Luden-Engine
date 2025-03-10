#ifndef MESH_HLSL
#define MESH_HLSL

#include "Mesh.hlsli"

#define ROOT_SIG	"RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |"\
					"DENY_HULL_SHADER_ROOT_ACCESS |"\
					"DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
					"DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
					"CBV(b0, space=0), "\
					"RootConstants(num32BitConstants=3, b1), "\
					"RootConstants(num32BitConstants=4, b2), "\
					"SRV(t0), "\
					"SRV(t1), "\
					"StaticSampler(s0, "\
                             "addressU = TEXTURE_ADDRESS_WRAP, "\
                             "filter = FILTER_MAXIMUM_ANISOTROPIC )"

static const uint MAX_TRIANGLES = 124;
static const uint MAX_VERTICES = 64;

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
	uint bDrawMeshlets;
};

struct VertexOffsets
{
	float3 PositionOffset;
	float2 TexCoordOffset;
	float3 NormalOffset;
	float4 TangentOffset;
};

struct VertexOut
{
	float4	Position		: SV_POSITION;
	float4	WorldPosition	: WORLD_POSITION;
	float2	TexCoord		: TEXCOORD;
	float3	Normal			: NORMAL;
	float4	Tangent			: TANGENT;
	uint	MeshletIndex	: COLOR0;
};

struct VertexData
{
	uint Index;
};

ConstantBuffer<Transform> Transforms : register(b0);
ConstantBuffer<PushConstants> Constants : register(b1);
ConstantBuffer<VertexOffsets> Vertex : register(b2);
StructuredBuffer<uint> UniqueVertexIndices : register(t0);
StructuredBuffer<uint> PrimitiveIndices : register(t1);

uint Hash(uint Value)
{
	Value = (Value + 0x7ed55d16) + (Value << 12);
	Value = (Value ^ 0xc761c23c) ^ (Value >> 19);
	Value = (Value + 0x165667b1) + (Value << 5);
	Value = (Value + 0xd3a2646c) ^ (Value << 9);
	Value = (Value + 0xfd7046c5) + (Value << 3);
	Value = (Value ^ 0xb55a4f09) ^ (Value >> 16);
	return Value;
}

// Get Triangle from given location and unpack it.
uint3 GetPrimitive(uint Location)
{
	uint packed = PrimitiveIndices[Location];
	
	uint idx0 = (packed) & 0x3FF;
	uint idx1 = (packed >> 10) & 0x3FF;
	uint idx2 = (packed >> 20) & 0x3FF;
	
	return uint3(idx0, idx1, idx2);
}

float3 GetMeshletColor(uint MeshletIndex)
{
	uint mhash = Hash(MeshletIndex);
	
	return float3(
		float(mhash & 255),
		float((mhash >> 8) & 255),
		float((mhash >> 16) & 255)) / 255.0;

	//return float3(float(MeshletIndex & 1),
	//		float(MeshletIndex & 3) / 4,
	//		float(MeshletIndex & 7) / 8);
}

VertexOut GetVertexAttributes(uint VertexIndex, uint MeshletIndex)
{
	ByteAddressBuffer vertexBuffer = ResourceDescriptorHeap[Constants.VertexIndex];

	float3 position = vertexBuffer.Load<float3>(Vertex.PositionOffset + (VertexIndex * 12));
	float2 texCoord = vertexBuffer.Load<float2>(Vertex.TexCoordOffset + (VertexIndex * 8));
	float3 normal	= vertexBuffer.Load<float3>(Vertex.NormalOffset + (VertexIndex * 12));
	float4 tangent	= vertexBuffer.Load<float4>(Vertex.TangentOffset + (VertexIndex * 16));
	
	VertexOut vout;
	
	vout.Position = mul(Transforms.WVP, float4(position, 1.0f));
	vout.TexCoord = texCoord;
	vout.Normal = mul((float3x3) Transforms.World, normal);
	vout.Tangent = mul(Transforms.World, tangent);
	vout.MeshletIndex = MeshletIndex;
	
	return vout;
}

[RootSignature(ROOT_SIG)]
[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint GroupID : SV_GroupID,
	out indices uint3 Triangles[MAX_TRIANGLES],
	out vertices VertexOut Vertices[MAX_VERTICES])
{
	StructuredBuffer<Meshlet> Meshlets = ResourceDescriptorHeap[Constants.MeshletIndex];
	
	Meshlet meshlet = Meshlets[GroupID];

	SetMeshOutputCounts(meshlet.VertexCount, meshlet.TriangleCount);

	if (GroupThreadID < meshlet.TriangleCount)
	{
		Triangles[GroupThreadID] = GetPrimitive(meshlet.TriangleOffset + GroupThreadID);
	}
	
	if (GroupThreadID < meshlet.VertexCount)
	{
		uint vertexIndex = UniqueVertexIndices[meshlet.VertexOffset + GroupThreadID];
		Vertices[GroupThreadID] = GetVertexAttributes(vertexIndex, GroupID);
	}
}

float4 PSMain(VertexOut pin) : SV_TARGET
{
	float3 output = pin.Normal;
	
	if (Constants.bDrawMeshlets)
	{
		float3 meshletColor = GetMeshletColor(pin.MeshletIndex);
		return float4(meshletColor, 1.0f);
	}

	return float4(output, 1.0f);
}

#endif // MESH_HLSL

#ifndef MESH_HLSLI
#define MESH_HLSLI

#define MAX_TRIANGLES	124
#define MAX_VERTICES	64

struct Meshlet
{
	uint VertexOffset;
	uint TriangleOffset;
	uint VertexCount;
	uint TriangleCount;
};

struct MeshletBound
{
	float3	Center;
	float	Radius;

	float3	ConeApex;
	float3	ConeAxis;
	// cos(angle/2)
	float	ConeCutoff;
};

uint3 UnpackTriangle(uint Packed)
{
	uint idx0 = (Packed		 ) & 0xFF;
	uint idx1 = (Packed >> 8 ) & 0xFF;
	uint idx2 = (Packed >> 16) & 0xFF;
	
	return uint3(idx0, idx1, idx2);
}

uint Hash(uint Value)
{
	Value = (Value + 0x7ed55d16) + (Value << 12);
	Value = (Value ^ 0xc761c23c) ^ (Value >> 19);
	Value = (Value + 0x165667b1) + (Value << 5 );
	Value = (Value + 0xd3a2646c) ^ (Value << 9 );
	Value = (Value + 0xfd7046c5) + (Value << 3 );
	Value = (Value ^ 0xb55a4f09) ^ (Value >> 16);
	
	return Value;
}

float3 GetMeshletColor(uint MeshletIndex)
{
	return float3(
		float(MeshletIndex & 1),
		float(MeshletIndex & 3) / 4,
		float(MeshletIndex & 7) / 8);
}

float3 GetMeshletColorHashed(uint MeshletIndex)
{
	uint mhash = Hash(MeshletIndex);
	
	return float3(
		float((mhash	  ) & 255),
		float((mhash >> 8 ) & 255),
		float((mhash >> 16) & 255)) / 255.0;
}

#endif // MESH_HLSLI

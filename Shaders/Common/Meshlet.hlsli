#ifndef MESHLET_HLSLI
#define MESHLET_HLSLI

// Note:
// if using meshoptimizer meshlet should be structured as follows:
//		- VertexOffset;
// 		- TriangleOffset;
// 		- VertexCount;
// 		- TriangleCount;
// yet for using DirectXMesh structured as follows:
// 		- VertexCount;
// 		- TriangleCount;
//		- VertexOffset;
// 		- TriangleOffset;

struct Meshlet
{
	uint VertexOffset;
	uint TriangleOffset;
	uint VertexCount;
	uint TriangleCount;
};

struct MeshletBounds
{
	/* bounding sphere, useful for frustum and occlusion culling */
	float Center[3];
	float Radius;

	/* normal cone, useful for backface culling */
	float ConeApex[3];
	float ConeAxis[3];
	float ConeCutoff; /* = cos(angle/2) */

};

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

#endif // MESHLET_HLSLI

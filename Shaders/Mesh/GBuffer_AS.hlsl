#ifndef GBUFFER_AS_HLSL
#define GBUFFER_AS_HLSL

#include "GBufferCommon.hlsli"

bool IsVisible(MeshletBound bounds, row_major float4x4 world, float3 viewPos)
{
	// Frustum culling.
	float4 center = mul(world, float4(bounds.Center.xyz, 1));
	float radius = bounds.Radius;

	for (int i = 0; i < 6; ++i)
	{
		if (dot(center.xyz, CameraConstants.Planes[i].xyz) + CameraConstants.Planes[i].w <= -radius)
		{
			return false;
		}
	}
	
	// Backface culling
	if (dot(normalize(bounds.ConeApex - viewPos), bounds.ConeAxis) >= bounds.ConeCutoff)
	{
		return false;
	}
	
	return true;
}

groupshared Payload sPayload;

[NumThreads(AS_GROUP_SIZE, 1, 1)]
void ASMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint DispatchThreadID : SV_DispatchThreadID,
	uint GroupID : SV_GroupID)
{
	StructuredBuffer<MeshletBound> MeshletBoundsBuffer = ResourceDescriptorHeap[Constants.MeshletBoundsIndex];
	MeshletBound bounds = MeshletBoundsBuffer[DispatchThreadID];
	
	bool visible = IsVisible(bounds, Transforms.World, CameraConstants.Position);
	
	if (visible)
	{
		uint index = WavePrefixCountBits(visible);
		sPayload.MeshletIndices[index] = DispatchThreadID;
	}
	
	uint visibleCount = WaveActiveCountBits(visible);
	DispatchMesh(visibleCount, 1, 1, sPayload);
	
	// Non-culling version.
	/*
	sPayload.MeshletIndices[GroupThreadID] = DispatchThreadID;
	DispatchMesh(AS_GROUP_SIZE, 1, 1, sPayload);
	*/
}

#endif // GBUFFER_AS_HLSL

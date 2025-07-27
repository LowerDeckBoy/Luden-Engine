#ifndef GBUFFER_AS_HLSL
#define GBUFFER_AS_HLSL

#include "GBufferCommon.hlsli"

bool IsVisible(MeshletBound bounds, row_major float4x4 world, float3 viewPos)
{
	// Frustum culling.
	float4 center = mul(world, float4(bounds.Center.xyz, 1.0f));

	for (int i = 0; i < 6; ++i)
	{
		if (dot(CameraConstants.Planes[i].xyz, center.xyz) + CameraConstants.Planes[i].w <= -bounds.Radius)
		{
			return false;
		}
	}
	
	// Backface culling
	//if (dot(normalize(bounds.ConeApex - viewPos), bounds.ConeAxis) >= bounds.ConeCutoff)
	//{
	//	return false;
	//}
	
	return true;
}

static StructuredBuffer<MeshletBound>	MeshletBoundsBuffer = ResourceDescriptorHeap[Constants.MeshletBoundsIndex];
static StructuredBuffer<Transform>		TransformBuffer		= ResourceDescriptorHeap[Constants.TransformBuffer];

groupshared Payload sPayload;

[NumThreads(AS_GROUP_SIZE, 1, 1)]
void ASMain(
	uint GroupThreadID : SV_GroupThreadID,
	uint DispatchThreadID : SV_DispatchThreadID,
	uint GroupID : SV_GroupID)
{
	uint dispatchCount = AS_GROUP_SIZE;

	if (Constants.bMeshletCulling)
	{
		MeshletBound bounds = MeshletBoundsBuffer[DispatchThreadID];
		Transform transform = TransformBuffer[Constants.TransformID];
		
		bool visible = IsVisible(bounds, transform.World, CameraConstants.Position);

		if (visible)
		{
			uint index = WavePrefixCountBits(visible);
			sPayload.MeshletIndices[index] = DispatchThreadID;
		}
	
		dispatchCount = WaveActiveCountBits(visible);

	}
	else
	{
		sPayload.MeshletIndices[GroupThreadID] = DispatchThreadID;
	}
		
	DispatchMesh(dispatchCount, 1, 1, sPayload);
	
	
	// Non-culling version.
	/*
	sPayload.MeshletIndices[GroupThreadID] = DispatchThreadID;
	DispatchMesh(AS_GROUP_SIZE, 1, 1, sPayload);
	*/
}

#endif // GBUFFER_AS_HLSL

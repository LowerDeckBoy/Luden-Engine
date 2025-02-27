#ifndef CAMERA_HLSLI
#define CAMERA_HLSLI

struct CameraProperties
{
	float4x4 View;
	float4x4 Projection;
	float4x4 InversedView;
	float4x4 InversedProjection;

	float3 Position;
	float pad0;
	
	float NearZ;
	float FarZ;
	// Placeholder
	float FoV;
	float pad1;
	
};

#endif // CAMERA_HLSLI
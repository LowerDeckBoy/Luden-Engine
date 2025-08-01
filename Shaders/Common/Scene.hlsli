#ifndef SCENE_HLSLI
#define SCENE_HLSLI

struct SceneConstants
{
	row_major float4x4	View;
	row_major float4x4	Projection;
	row_major float4x4	InversedView;
	row_major float4x4	InversedProjection;
	row_major float4x4	InversedViewProjection;
	float3				CameraPosition;
	float				pad;
	uint				Width;
	uint				Height;
	float				AspectRatio;
	float				pad2;
	
	float4				Planes[6];
	
	float3				DirectionalPosition;
	float				DirectionalIntensity;
	float3				DirectionalAmbient;
	float				pad3;
};

#endif // SCENE_HLSLI

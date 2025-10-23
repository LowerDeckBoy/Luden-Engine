#ifndef SCENE_HLSLI
#define SCENE_HLSLI

struct SceneConstants
{
	float4x4	View;
	float4x4	Projection;
	float4x4	InversedView;
	float4x4	InversedProjection;
	float4x4	InversedViewProjection;
	float3		CameraPosition;
	float		pad;
	uint		Width;
	uint		Height;
	float		AspectRatio;
	float		pad2;
	
	float4		Planes[6];
	
	float3		DirectionalPosition;
	float		DirectionalIntensity;
	float3		DirectionalAmbient;
	float		pad3;
};

#endif // SCENE_HLSLI

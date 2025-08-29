#ifndef SSR_HLSL
#define SSR_HLSL

#include "SSR_RS.hlsli"
#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"

#define DISPATCH_BLOCK 16
//https://www.reddit.com/r/GraphicsProgramming/comments/1n2s26j/how_can_i_convert_depth_buffer_to_world_pos_in/
struct SSRParameters
{
	//float4x4 View;
	float4x4 Projection;
	row_major float4x4 InversedView;
	row_major float4x4 InversedProjection;

	uint InputImageIndex;
	uint OutputImageIndex;
	uint NormalIndex;
	uint RoughnessIndex;
	uint WorldPositionIndex;
	uint DepthIndex;

};

ConstantBuffer<SSRParameters> Constants : register(b0);

SamplerState LinearClampSampler : register(s0);
SamplerState PointClampSampler : register(s1);


float unprojectDepth(float depth, float near, float far)
{
    return near * far / (far - depth * (far - near));
}

float3 GetViewPosition(float2 UV, float Depth)
{
	float2 ndc = UV * 2.0f + 1.0f;
	float4 clipPos = float4(ndc, 1.0f, 1.0f);
	float4 viewPosH = mul(Constants.InversedProjection, clipPos);
	//float4 viewPosH = mul(clipPos, Constants.InversedProjection);
	float3 positionVS = viewPosH.xyz * Depth;
	//float4 worldPosH = mul(float4(positionVS, 1.0f), Constants.InversedView);
	float4 worldPosH = mul(Constants.InversedView, float4(positionVS, 1.0f));

	return worldPosH.xyz;
}

float3 PositionFromDepth(float2 UV, float Depth) 
{
   	float z = Depth;

    float4 clipSpacePosition = float4(UV * 2.0f - 1.0f, z, 1.0f);
	float4x4 viewProj = mul(Constants.InversedProjection, Constants.InversedView);
    float4 viewSpacePosition = mul(clipSpacePosition, viewProj);

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

const static uint SearchSteps = 16;

float4 BinarySearch(in Texture2D DepthTexture, float3 Ray, float RayThreshold, inout float3 ViewHitCoords)
{
	float depth;

    [unroll(SearchSteps)]
    for (int i = 0; i < SearchSteps; i++)
    {
      	float4 projectedCoords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
        projectedCoords.xy /= projectedCoords.w;
        projectedCoords.xy = projectedCoords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

       //linearize depth here
        depth = DepthTexture.SampleLevel(PointClampSampler, projectedCoords.xy, 0).r;
        float3 fPositionVS = GetViewPosition(projectedCoords.xy, depth);
        float fDepthDiff = ViewHitCoords.z - fPositionVS.z;

        if (fDepthDiff <= 0.0f)
            ViewHitCoords += Ray;

        Ray *= 0.5f;
        ViewHitCoords -= Ray;
    }

    float4 projectedCoords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
    projectedCoords.xy /= projectedCoords.w;
    projectedCoords.xy = projectedCoords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

    depth = DepthTexture.SampleLevel(PointClampSampler, projectedCoords.xy, 0).r;
    float3 viewPosition = GetViewPosition(projectedCoords.xy, depth);
    float depthDiff = ViewHitCoords.z - viewPosition.z;

    return float4(projectedCoords.xy, depth, abs(depthDiff) < RayThreshold ? 1.0f : 0.0f);
}

float4 RayMarch(in Texture2D DepthTexture, float3 Ray, float RayStep, float RayThreshold, inout float3 ViewHitCoords)
{
	float depth;

	for (uint i = 0; i < 16; ++i)
	{
		ViewHitCoords += Ray;

		float4 projectedCoords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
        projectedCoords.xy /= projectedCoords.w;
        projectedCoords.xy = projectedCoords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
		depth = DepthTexture.SampleLevel(PointClampSampler, projectedCoords.xy, 0).r;
        float3 viewPosition = GetViewPosition(projectedCoords.xy, depth);
        float depthDiff = ViewHitCoords.z - viewPosition.z;

		[branch]
        if (depthDiff > 0.0f)
        {
            return BinarySearch(DepthTexture, Ray, RayThreshold, ViewHitCoords);
        }
        Ray *= RayStep;
    }

    return 0.0f;
}


[RootSignature(SSR_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D sceneTexture 				= GetTexture(Constants.InputImageIndex);
	Texture2D normalTexture 			= GetTexture(Constants.NormalIndex);
	Texture2D roughnessTexture 			= GetTexture(Constants.RoughnessIndex);
	Texture2D worldTexture				= GetTexture(Constants.WorldPositionIndex);
	Texture2D depthTexture				= GetTexture(Constants.DepthIndex);
	RWTexture2D<float4> outputTexture 	= GetRWTexture<float4>(Constants.OutputImageIndex);

	float2 textureSize = GetTextureSize(normalTexture);
	float2 texelSize = GetTexelSize(textureSize);
	float2 uv = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	float depth = depthTexture[DispatchThreadID.xy].r;
	float linearDepth = unprojectDepth(depth, 0.01f, 10000.0f);
	float3 viewPosition = GetViewPosition(uv, depth);
	float3 position = worldTexture.Load(uint3(DispatchThreadID.xy, 0.0f)).xyz;
	//float3 position = PositionFromDepth(uv, linearDepth);
	
	float roughness = roughnessTexture.Load(uint3(DispatchThreadID.xy, 0.0f)).g;
	//float roughness = 0.0f;

	float3 normals = normalTexture[DispatchThreadID.xy].rgb;
 	float3 reflectDir = normalize(reflect(viewPosition, normals));

	float4 hitCoords = RayMarch(depthTexture, reflectDir, 1.6f, 2.0f, viewPosition);
	//outputTexture[DispatchThreadID.xy] = float4(viewPosition.xyz, 1.0f);
	//return;
	//outputTexture[DispatchThreadID.xy] = float4(hitCoords.xyz, 1.0f);

 	float2 coordsEdgeFactors = float2(1, 1) - pow(saturate(abs(hitCoords.xy - float2(0.5f, 0.5f)) * 2), 8);
    float  screenEdgeFactor = saturate(min(coordsEdgeFactors.x, coordsEdgeFactors.y));

	float3 sceneColor = sceneTexture.Load(uint3(DispatchThreadID.xy, 0.0f)).rgb;
	if (roughness >= 0.7f)
   	{
   	    outputTexture[DispatchThreadID.xy] = float4(sceneColor, 1.0f);
   	    return;
   	}

    float3 hitColor = sceneTexture.SampleLevel(LinearClampSampler, hitCoords.xy, 0).rgb;
	//outputTexture[DispatchThreadID.xy] = float4(hitColor, 1.0f);
	//return;
	
    float roughnessMask = saturate(1.0f - (roughness / 0.7f));
    roughnessMask *= roughnessMask;
    float4 fresnel = clamp(pow(1 - dot(normalize(viewPosition), normals), 1), 0, 1);

    float4 reflectionColor = float4(saturate(hitColor.xyz * screenEdgeFactor * roughnessMask), 1.0f);
	
	outputTexture[DispatchThreadID.xy] = float4(sceneColor + fresnel.xyz * max(0, reflectionColor.xyz), 1.0f);
	//outputTexture[DispatchThreadID.xy] = float4(normals, 1.0f);
	//outputTexture[DispatchThreadID.xy] = float4(reflectionColor.xyz, 1.0f);
	//outputTexture[DispatchThreadID.xy] = float4(reflectDir, 1.0f);
	//outputTexture[DispatchThreadID.xy] = float4(position, 1.0f);
	//outputTexture[DispatchThreadID.xy] = float4(z, z, z, 1.0f);
}

#endif // SSR_HLSL
#ifndef SSR_HLSL
#define SSR_HLSL

#include "SSR_RS.hlsli"
#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"

#define DISPATCH_BLOCK 16

struct SSRParameters
{
	row_major float4x4 Projection;
	float4x4 InversedView;
	float4x4 InversedProjection;

	uint InputImageIndex;
	uint OutputImageIndex;
	uint NormalIndex;
	uint RoughnessIndex;
	uint WorldPositionIndex;
	uint DepthIndex;

	float RaySteps;
	float RayThreshold;
};

ConstantBuffer<SSRParameters> Constants : register(b0);

SamplerState LinearClampSampler : register(s0);
SamplerState PointClampSampler : register(s1);


float UnprojectDepth(float Depth, float Near, float Far)
{
    return Near * Far / (Far - Depth * (Far - Near));
}

float3 GetViewPosition(float2 UV, float Depth)
{
	UV.y = 1.0f - UV.y;
	float2 ndc = UV * 2.0f - 1.0f;
	float4 clipPos = float4(ndc, Depth, 1.0f);

	float4 viewPosH = mul(clipPos, Constants.InversedProjection);

	return viewPosH.xyz / viewPosH.w;
}

const static uint SearchSteps = 16;

float4 BinarySearch(inout Texture2D DepthTexture, float3 Ray, float RayThreshold, inout float3 ViewHitCoords)
{
	float depth;

    [unroll(SearchSteps)]
    for (int i = 0; i < SearchSteps; i++)
    {
      	//float4 coords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
      	float4 coords = mul(Constants.Projection, float4(ViewHitCoords, 1.0f));
        coords.xy /= coords.w;
        coords.xy = coords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

        depth = 1.0f - DepthTexture.Sample(PointClampSampler, coords.xy).r;
		depth = depth * 2.0f - 1.0f;
        float3 fPositionVS = GetViewPosition(coords.xy, depth);
        float fDepthDiff = ViewHitCoords.z - fPositionVS.z;

        if (fDepthDiff <= 0.0f)
            ViewHitCoords += Ray;

        Ray *= 0.5f;
        ViewHitCoords -= Ray;
    }

  	//float4 projectedCoords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
  	float4 projectedCoords = mul(Constants.Projection, float4(ViewHitCoords, 1.0f));
    projectedCoords.xy /= projectedCoords.w;
    projectedCoords.xy = projectedCoords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

    depth = 1.0f - DepthTexture.Sample(PointClampSampler, projectedCoords.xy).r;
    float3 viewPosition = GetViewPosition(projectedCoords.xy, depth);
    float depthDiff = ViewHitCoords.z - viewPosition.z;

    return float4(projectedCoords.xy, depth, abs(depthDiff) < RayThreshold ? 1.0f : 0.0f);
}

float4 RayMarch(inout Texture2D DepthTexture, float3 Ray, float RayStep, float RayThreshold, inout float3 ViewHitCoords)
{
	float depth;

	for (uint i = 0; i < SearchSteps; ++i)
	{
		ViewHitCoords += Ray;

		//float4 projectedCoords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
		float4 projectedCoords = mul(Constants.Projection, float4(ViewHitCoords, 1.0f));
        projectedCoords.xy /= projectedCoords.w;
        projectedCoords.xy = projectedCoords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
		depth = DepthTexture.Sample(PointClampSampler, projectedCoords.xy).r;
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
	Texture2D depthTexture				= GetTexture(Constants.DepthIndex);
	RWTexture2D<float4> outputTexture 	= GetRWTexture<float4>(Constants.OutputImageIndex);

	float2 textureSize = GetTextureSize(normalTexture);
	float2 texelSize = GetTexelSize(textureSize);
	float2 uv = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	float4 sceneColor = sceneTexture.Sample(LinearClampSampler, uv);
	float depth = 1.0f - depthTexture[DispatchThreadID.xy].r;

	if (depth <= 0.0001f)
   	{
		outputTexture[DispatchThreadID.xy] = float4(sceneColor);
		return;
   	}

	float roughness = roughnessTexture.Load(uint3(DispatchThreadID.xy, 0.0f)).g;
	if (roughness >= 0.7f)
   	{
   	    outputTexture[DispatchThreadID.xy] = float4(sceneColor);
   	    return;
   	}

	float3 viewPosition = GetViewPosition(uv, depth);

	float3 normals 		= normalTexture.Load(uint3(DispatchThreadID.xy, 0.0f)).rgb;
 	float3 reflectDir 	= normalize(reflect(viewPosition, normals));
	float4 hitCoords 	= RayMarch(depthTexture, reflectDir, Constants.RaySteps, Constants.RayThreshold, viewPosition);

 	float2 coordsEdgeFactors = float2(1, 1) - pow(saturate(abs(hitCoords.xy - float2(0.5f, 0.5f)) * 2), 8);
    float  screenEdgeFactor = saturate(min(coordsEdgeFactors.x, coordsEdgeFactors.y));

    float3 hitColor = sceneTexture.Sample(LinearClampSampler, hitCoords.xy).rgb;
	
   	float roughnessMask = saturate(1.0f - (roughness / 0.7f));
    roughnessMask *= roughnessMask;

    float4 fresnel = clamp(pow(1 - dot(normalize(viewPosition), normals), 1.0f), 0.0f, 1.0f);

    float4 reflectionColor = float4(saturate(hitColor.xyz * screenEdgeFactor * roughnessMask), 1.0f);
	
	outputTexture[DispatchThreadID.xy] = float4(sceneColor +  max(0, reflectionColor));
}

#endif // SSR_HLSL
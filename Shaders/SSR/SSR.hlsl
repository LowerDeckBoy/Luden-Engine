#ifndef SSR_HLSL
#define SSR_HLSL

#include "SSR_RS.hlsli"
#include "../Common/Common.hlsli"
#include "../Common/Bindless.hlsli"

#define DISPATCH_BLOCK 16

struct SSRParameters
{
	float4x4 Projection;
	float4x4 InversedProjection;

	uint	 InputImageIndex;
	uint	 OutputImageIndex;
	uint	 NormalIndex;
	uint	 RoughnessIndex;
	uint	 DepthIndex;
	float	 RaySteps;
	float	 RayThreshold;
	float	 padding;
};

ConstantBuffer<SSRParameters> Constants : register(b0);

SamplerState LinearClampSampler : register(s0);
SamplerState PointClampSampler  : register(s1);

const static uint SearchSteps = 16;

float4 BinarySearch(inout Texture2D DepthTexture, inout float3 ViewRay, inout float3 ViewHitCoords)
{
	float depth;

	for (int i = 0; i < SearchSteps; i++)
	{
		float4 coords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
		coords.xy /= coords.w;
		coords.xy = coords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

		depth = DepthTexture.Sample(LinearClampSampler, coords.xy).r;
		
		float3 fPositionVS = GetViewPosition(coords.xy, depth, transpose(Constants.InversedProjection));
		float fDepthDiff = ViewHitCoords.z - fPositionVS.z;

		if (fDepthDiff <= 0.0f)
		{
			ViewHitCoords += ViewRay;
		}

		ViewRay *= 0.5f;
		ViewHitCoords -= ViewRay;
	}

	float4 projectedCoords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
	projectedCoords.xy /= projectedCoords.w;
	projectedCoords.xy = projectedCoords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

	depth = DepthTexture.Sample(LinearClampSampler, projectedCoords.xy).r;
	float3 viewPosition = GetViewPosition(projectedCoords.xy, depth, transpose(Constants.InversedProjection));
	float depthDiff = ViewHitCoords.z - viewPosition.z;

	return float4(projectedCoords.xy, depth, abs(depthDiff) < Constants.RayThreshold ? 1.0f : 0.0f);
}

float4 RayMarch(inout Texture2D DepthTexture, inout float3 ViewRay, inout float3 ViewHitCoords)
{
	float depth;

	for (uint i = 0; i < SearchSteps; i++)
	{
		ViewHitCoords += ViewRay;

		float4 projectedCoords = mul(float4(ViewHitCoords, 1.0f), Constants.Projection);
		projectedCoords.xy /= projectedCoords.w;
		projectedCoords.xy = projectedCoords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

		depth = DepthTexture.Sample(LinearClampSampler, projectedCoords.xy).r;
		float3 viewPosition = GetViewPosition(projectedCoords.xy, depth, transpose(Constants.InversedProjection));
		float depthDiff = ViewHitCoords.z - viewPosition.z;

		[branch]
		if (depthDiff > 0.0f)
		{
			return BinarySearch(DepthTexture, ViewRay, ViewHitCoords);
		}

		ViewRay *= Constants.RaySteps;
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

	float2 textureSize = GetTextureSize(sceneTexture);
	float2 texelSize = GetTexelSize(textureSize);
	float2 uv = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;
	
	if (textureSize.x <= DispatchThreadID.x || textureSize.y <= DispatchThreadID.y)
	{
		return;
	}

	float4 sceneColor = sceneTexture.Sample(LinearClampSampler, uv);

	float depth = depthTexture.Sample(LinearClampSampler, uv).r;
	if (depth <= 0.00001f)
	{
		outputTexture[DispatchThreadID.xy] = float4(sceneColor);
		return;
	}
	
	float roughness = roughnessTexture.Sample(LinearClampSampler, uv).g;
	if (roughness >= 0.7f)
	{
		outputTexture[DispatchThreadID.xy] = float4(sceneColor);
		return;
	}

	float3 viewPosition = GetViewPosition(uv, depth, transpose(Constants.InversedProjection));

	float3 normals 		= normalize(normalTexture.Sample(LinearClampSampler, uv).rgb * 2.0f - 1.0f);
	float3 reflectDir 	= normalize(reflect(viewPosition, normals));
	float4 hitCoords 	= RayMarch(depthTexture, reflectDir, viewPosition);
	
	float2 coordsEdgeFactors = float2(1.0f, 1.0f) - pow(saturate(abs(hitCoords.xy - float2(0.5f, 0.5f)) * 2.0f), 8.0f);
	float  screenEdgeFactor = saturate(min(coordsEdgeFactors.x, coordsEdgeFactors.y));

	float3 hitColor = sceneTexture.Sample(LinearClampSampler, hitCoords.xy).rgb;
	
	float roughnessMask = saturate(1.0f - (roughness / 0.7f));
	roughnessMask *= roughnessMask;

	float4 fresnel = clamp(pow(1.0f - dot(normalize(viewPosition), normals), 1.0f), 0.0f, 1.0f);

	float4 reflectionColor = float4(saturate(hitColor.xyz * screenEdgeFactor * roughnessMask), 1.0f);

	outputTexture[DispatchThreadID.xy] = float4(sceneColor + fresnel * max(0.0f, reflectionColor));
}

#endif // SSR_HLSL
#ifndef DEFERRED_HLSL
#define DEFERRED_HLSL

#include "Deferred_RS.hlsli"
#include "../Common/Light.hlsli"
#include "../Common/Common.hlsli"

struct PushConstants
{
	uint LightBufferIndex;
	uint NumLights;
	uint BaseColorIndex;
	uint NormalIndex;
	uint MRIndex;
	uint EmissiveIndex;
};

ConstantBuffer<PushConstants> Constants : register(b2);

struct SceneConstants
{
	row_major float4x4 World;
	row_major float4x4 View;
	row_major float4x4 Projection;
	row_major float4x4 InversedView;
	row_major float4x4 InversedProjection;
	row_major float4x4 InversedViewProjection;
	float3 CameraPosition;
	float pad;
	float4 Planes[6];
};

ConstantBuffer<SceneConstants> Scene : register(b1);

float3 GetWorldPosition(float Depth, float2 UV, row_major float4x4 InvView, row_major float4x4 InvProjection)
{
	const float z = Depth * 2.0f - 1.0f;
	
	float4 clipSpacePos = float4(UV, Depth, 1.0f);
	float4 viewSpacePos = mul(InvProjection, clipSpacePos);
	//float4 viewSpacePos = mul(clipSpacePos, InvProjection);
	viewSpacePos.xyz /= viewSpacePos.w;
	//return viewSpacePos.xyz;
	float4 worldSpacePosition = mul(InvView, viewSpacePos);
	//float4 worldSpacePosition = mul(viewSpacePos, InvView);

	return worldSpacePosition.xyz;
}

float3 GetWorldPosition(float Depth, float2 UV,  float4x4 InvViewProj)
{
	const float z = Depth;
	
	float4 clipPos = float4(UV, z, 1.0);

	//float4 worldPos = mul(InvViewProj, clipPos);
	float4 worldPos = mul(clipPos, InvViewProj);
	worldPos.xyz /= worldPos.w;

	return worldPos.xyz;
}

struct ScreenQuadOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

ScreenQuadOutput VSMain(uint VertexID : SV_VertexID)
{
	ScreenQuadOutput output = (ScreenQuadOutput) 0;
	
	output.TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	output.Position = float4(output.TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
	output.Position.y *= -1.0f;
	
	return output;
}

[RootSignature(ROOT_SIG)]
float4 PSMain(ScreenQuadOutput pin) : SV_TARGET0
{
	float3 output = float3(0.0f, 0.0f, 0.0f);

	const float2 uv = pin.Position.xy;
	
	Texture2D texBaseColor	= ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D texNormal		= ResourceDescriptorHeap[Constants.NormalIndex];
	Texture2D texMR			= ResourceDescriptorHeap[Constants.MRIndex];
	Texture2D texEmissive	= ResourceDescriptorHeap[Constants.EmissiveIndex];
	
	float4 baseColor				= texBaseColor.Load(int3(uv, 0.0f));
	const float4 normal				= texNormal.Load(int3(uv, 0.0f));
	const float3 metallicRoughness	= texMR.Load(int3(uv, 0.0f)).rgb;
	const float3 emissive			= texEmissive.Load(int3(uv, 0.0f)).rgb;
	
	const float metalness			= metallicRoughness.b;
	const float roughness			= metallicRoughness.g;
	
	StructuredBuffer<PointLight> PointLights = ResourceDescriptorHeap[Constants.LightBufferIndex];
	
	
	
	for (uint lightIdx = 0; lightIdx < Constants.NumLights; ++lightIdx)
	{
		PointLight light = PointLights[lightIdx];
		
		//const float L = light.Position;
		
		//const float NdotL = max(dot())
		
		//baseColor += float3(0.25f, 0.0f, 0.0f);
		baseColor.x += 0.25f;
		baseColor.y += 0.1f;
		baseColor.z += 0.11f;

	}
	
	//float3 worldPosition = GetWorldPosition(normal.w, uv, Scene.InversedView, Scene.InversedProjection);
	//float3 worldPosition = GetWorldPosition(normal.w, uv, Scene.InversedViewProjection);
	float3 worldPosition = float3(normal.w, normal.w, normal.w);
	
	return float4(worldPosition, 1.0f);
	//return float4(baseColor.rgb, 1.0f);
	//return float4(pin.TexCoord, 0.0f, 1.0f);
}

#endif // DEFERRED_HLSL

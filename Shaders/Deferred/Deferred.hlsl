#ifndef DEFERRED_HLSL
#define DEFERRED_HLSL

#include "Deferred_RS.hlsli"
#include "../PBR.hlsli"
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
	uint WorldPositionIndex;
	uint padding;
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
	const float2 uv = pin.Position.xy;
	
	Texture2D<float4> texBaseColor		= ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D<float4> texNormal			= ResourceDescriptorHeap[Constants.NormalIndex];
	Texture2D<float4> texMR				= ResourceDescriptorHeap[Constants.MRIndex];
	Texture2D<float4> texEmissive		= ResourceDescriptorHeap[Constants.EmissiveIndex];
	Texture2D<float4> texWorldPositon	= ResourceDescriptorHeap[Constants.WorldPositionIndex];
	
	const float4 baseColor			= texBaseColor.Load(int3(uv, 0.0f));
	const float4 normal				= texNormal.Load(int3(uv, 0.0f));
	const float3 metallicRoughness	= texMR.Load(int3(uv, 0.0f)).rgb;
	const float3 emissive			= texEmissive.Load(int3(uv, 0.0f)).rgb;
	const float3 worldPosition		= texWorldPositon.Load(int3(uv, 0.0f)).rgb;
	
	const float metalness			= metallicRoughness.b;
	const float roughness			= metallicRoughness.g;
	
	StructuredBuffer<PointLight> PointLights = ResourceDescriptorHeap[Constants.LightBufferIndex];
	
	float3 N = normal.rgb;
	const float  depth = normal.w;
	
	const float3 V = normalize(Scene.CameraPosition - worldPosition);
	const float NdotV = max(dot(N, V), Epsilon);
	
	const float3 Fdielectric = float3(0.04f, 0.04f, 0.04f);
	const float3 F0 = lerp(Fdielectric, baseColor.rgb, float3(metalness, metalness, metalness));

	float3 F = FresnelSchlick(NdotV, F0);
	float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - metalness);

	float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
	float3 output = ambient;

	for (uint lightIdx = 0; lightIdx < Constants.NumLights; lightIdx++)
	{
		PointLight light = PointLights[lightIdx];
		
		float3 L = normalize(light.Position - worldPosition);
		float3 H = normalize(V + L);
		
		float NdotL = max(dot(N, L), Epsilon);
		float NdotH = max(dot(N, H), Epsilon);
		float HdotV = max(dot(H, V), Epsilon);
		
		float distance = length(light.Position - worldPosition);
		float attenuation = 1.0f / (distance * distance + 1.0f);
		float3 radiance = attenuation * light.Ambient * saturate(1.0f - distance / light.Range);
		
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(NdotV, NdotL, roughness);

		float3 numerator = NDF * G * F;
		float denominator = 4.0f * NdotL * NdotV;
		float3 specular = numerator / (denominator + Epsilon);
		float3 diffuse = kD * baseColor.rgb / PI;
			
		output += (diffuse + specular) * NdotL * radiance * light.Ambient;
	}

	return float4(output.rgb, 1.0f);
}

#endif // DEFERRED_HLSL

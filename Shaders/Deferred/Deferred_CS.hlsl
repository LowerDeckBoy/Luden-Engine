#ifndef DEFERRED_CS_HLSL
#define DEFERRED_CS_HLSL

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
	uint TextureOutputIndex;
	//uint padding;
};

ConstantBuffer<PushConstants> Constants : register(b2);

struct SceneConstants
{
	row_major float4x4 View;
	row_major float4x4 Projection;
	row_major float4x4 InversedView;
	row_major float4x4 InversedProjection;
	row_major float4x4 InversedViewProjection;
	float3 CameraPosition;
	float pad;
	float4 Planes[6];
	
	float3 DirectionalPosition;
	float pad2;
	float3 DirectionalAmbient;
	float pad3;
};

ConstantBuffer<SceneConstants> Scene : register(b1);

SamplerState texSampler : register(s0);

[RootSignature(DEFERRED_ROOT_SIG)]
[numthreads(8, 8, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[Constants.TextureOutputIndex];

	float2 textureSize;
	outputTexture.GetDimensions(textureSize.x, textureSize.y);
	
	if (textureSize.x <= DispatchThreadID.x || textureSize.y <= DispatchThreadID.y)
	{
		return;
	}

	const float2 uv = DispatchThreadID.xy;
	
	Texture2D<float4> texBaseColor		= ResourceDescriptorHeap[Constants.BaseColorIndex];
	Texture2D<float4> texNormal			= ResourceDescriptorHeap[Constants.NormalIndex];
	Texture2D<float4> texMR				= ResourceDescriptorHeap[Constants.MRIndex];
	Texture2D<float4> texEmissive		= ResourceDescriptorHeap[Constants.EmissiveIndex];
	Texture2D<float4> texWorldPositon	= ResourceDescriptorHeap[Constants.WorldPositionIndex];
	
	const float4 baseColor = pow(texBaseColor.Load(int3(uv, 0.0f)), 2.2f);
	const float4 normal = texNormal.Load(int3(uv, 0.0f));
	const float3 metallicRoughness = texMR.Load(int3(uv, 0.0f)).rgb;
	const float3 emissive = texEmissive.Load(int3(uv, 0.0f)).rgb;
	const float3 worldPosition = texWorldPositon.Load(int3(uv, 0.0f)).rgb;

	const float metalness = metallicRoughness.b;
	const float roughness = metallicRoughness.g;
	
	StructuredBuffer<PointLight> PointLights = ResourceDescriptorHeap[Constants.LightBufferIndex];
	
	const float3 N = normalize(normal.rgb);
	const float depth = normal.w;
	
	const float3 V = normalize(Scene.CameraPosition - worldPosition);
	const float NdotV = max(dot(N, V), Epsilon);
	
	const float3 Fdielectric = float3(0.04f, 0.04f, 0.04f);
	const float3 F0 = lerp(Fdielectric, baseColor.rgb, float3(metalness, metalness, metalness));
	
	float3 output = float3(0.0f, 0.0f, 0.0f);

	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	
	// Point lights
	for (uint lightIdx = 0; lightIdx < Constants.NumLights; lightIdx++)
	{
		PointLight light = PointLights[lightIdx];
		
		const float3 L = normalize(light.Position - worldPosition);
		const float3 H = normalize(V + L);
		
		const float NdotL = max(dot(N, L), Epsilon);
		const float NdotH = max(dot(N, H), Epsilon);
		const float HdotV = max(dot(H, V), Epsilon);
		
		const float distance	= length(light.Position - worldPosition);
		const float attenuation = (1.0f / (distance * distance + 1.0f)) * (saturate(1.0f - distance / light.Radius));
		const float3 radiance	= attenuation * light.Ambient * light.Radius;
		
		const float D = DistributionGGX(N, H, roughness);
		const float G = GeometrySmith(NdotV, NdotL, roughness);
		const float3 F = FresnelSchlick(HdotV, F0);
		const float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - metalness);
		
		const float3 numerator = D * G * F;
		const float denominator = 4.0f * NdotL * NdotV + Epsilon;
		
		const float3 diffuse = kD * baseColor.rgb * InvPI;
		const float3 specular = numerator / denominator;

		Lo += (diffuse + specular) * radiance * NdotL;
	}
	
	// Single directional lighting.
	{
		const float3 L = -Scene.DirectionalPosition;
		const float3 H = normalize(V + L);
		
		const float NdotL = max(dot(N, L), Epsilon);
		const float NdotH = max(dot(N, H), Epsilon);
		const float HdotV = max(dot(H, V), Epsilon);

		const float D = DistributionGGX(N, H, roughness);
		const float G = GeometrySmith(NdotV, NdotL, roughness);
		const float3 F = FresnelSchlick(HdotV, F0);
		const float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - metalness);
		
		const float3 numerator = D * G * F;
		const float denominator = 4.0f * NdotL * NdotV + Epsilon;
		
		const float3 diffuse = kD * baseColor.rgb * InvPI;
		const float3 specular = numerator / denominator;

		Lo += (diffuse + specular) * pow(Scene.DirectionalAmbient, 2.2f) * NdotL;
	}

	output += Lo;
	output += emissive;
	// Reinhard
	output /= (output + float3(1.0f, 1.0f, 1.0f));
	// Gamma correction
	output = lerp(output, pow(output, 0.4545454545f), 0.4f);

	outputTexture[DispatchThreadID.xy] = float4(output.rgb, 1.0f);
}

#endif // DEFERRED_CS_HLSL

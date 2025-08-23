#ifndef LIGHT_HLSLI
#define LIGHT_HLSLI

#include "Common.hlsli"
#include "Color.hlsli"
#include "../PBR.hlsli"

struct PointLight
{
	float4x4	Transform;
	float3		Position;
	float		Intensity;
	float3		Ambient;
	float		Radius;
};

struct SpotLight
{
	float4x4	Transform;
	float3		Position;
	float		Intensity;
	float3		Direction;
	float		InnerCutoff;
	float3		Ambient;
	float		OuterCutoff;
};

float3 CalculatePointLight(PointLight Light, float3 BaseColor, float3 N, float3 V, float NdotV, float3 WorldPosition, float Metalness, float Roughness)
{
	const float3 L = normalize(Light.Position - WorldPosition);
	const float3 H = normalize(V + L);
		
	const float NdotL = max(dot(N, L), Epsilon);
	const float NdotH = max(dot(N, H), Epsilon);
	const float HdotV = max(dot(H, V), Epsilon);
		
	const float distance = length(Light.Position - WorldPosition);
	const float attenuation = (1.0f / (distance * distance + 1.0f)) * (saturate(1.0f - distance / Light.Radius));
	const float3 radiance = attenuation * Light.Ambient * Light.Radius;
		
	const float D = DistributionGGX(N, H, Roughness);
	const float G = GeometrySmith(NdotV, NdotL, Roughness);
	
	const float3 F0 = lerp(Fdielectric, BaseColor.rgb, float3(Metalness, Metalness, Metalness));
	const float3 F = FresnelSchlick(HdotV, F0);
	const float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - Metalness);
		
	const float3 numerator = D * G * F;
	const float denominator = 4.0f * NdotL * NdotV + Epsilon;
		
	const float3 diffuse = kD * BaseColor.rgb * InvPI;
	const float3 specular = numerator / denominator;

	return (diffuse + specular) * radiance * NdotL;
}

float3 CalculateDirectionalLight(float3 Direction, float3 Ambient, float Intensity, float3 BaseColor, float3 N, float3 V, float NdotV, float Metalness, float Roughness)
{
	const float3 L = -Direction;
	const float3 H = normalize(V + L);
		
	const float NdotL = max(dot(N, L), Epsilon);
	const float NdotH = max(dot(N, H), Epsilon);
	const float HdotV = max(dot(H, V), Epsilon);

	const float D = DistributionGGX(N, H, Roughness);
	const float G = GeometrySmith(NdotV, NdotL, Roughness);
	const float3 F0 = lerp(Fdielectric, BaseColor, float3(Metalness, Metalness, Metalness));
	const float3 F = FresnelSchlick(HdotV, F0);
	const float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - Metalness);
		
	const float3 numerator = D * G * F;
	const float denominator = 4.0f * NdotL * NdotV + Epsilon;
		
	const float3 diffuse = kD * BaseColor * InvPI;
	const float3 specular = numerator / denominator;

	return (diffuse + specular) * (Ambient * Intensity) * NdotL;
}

float3 CalculateSpotLight(SpotLight Light, float3 BaseColor, float3 N, float3 V, float NdotV, float3 WorldPosition, float Metalness, float Roughness)
{
	const float3 L = normalize(Light.Position - WorldPosition);
	const float3 H = normalize(V + L);
		
	float theta = dot(L, normalize(Light.Direction));
	
	float innerAngle = DegreesToRadians(Light.InnerCutoff);
	float outerAngle = DegreesToRadians(Light.OuterCutoff);
	
	if (innerAngle >= outerAngle)
	{
		return float3(0.0f, 0.0f, 0.0f);
	}
	
	float cosAngle = cos(outerAngle);
	float falloff = saturate((theta - cosAngle) / (1.0f - cosAngle));

	const float NdotL = max(dot(N, L), Epsilon);
	const float NdotH = max(dot(N, H), Epsilon);
	const float HdotV = max(dot(H, V), Epsilon);

	const float attenuation = smoothstep(innerAngle, outerAngle, theta);
	const float3 radiance	= attenuation * Light.Ambient * Light.Intensity;

	const float D = DistributionGGX(N, H, Roughness);
	const float G = GeometrySmith(NdotV, NdotL, Roughness);
	
	const float3 F0 = lerp(Fdielectric, BaseColor.rgb, float3(Metalness, Metalness, Metalness));
	const float3 F = FresnelSchlick(HdotV, F0);
	const float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - Metalness);
		
	const float3 numerator = D * G * F;
	const float denominator = 4.0f * NdotL * NdotV + Epsilon;
		
	const float3 diffuse = kD * BaseColor.rgb * InvPI;
	const float3 specular = numerator / denominator;

	return (diffuse + specular) * radiance * NdotL;
}

#endif // LIGHT_HLSLI

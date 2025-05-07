#ifndef PBR_HLSLI
#define PBR_HLSLI

#include "Common/Common.hlsli"

//
// N - surface's normal
// V - view vector (camera's eye)
// H - halfway vector
// Kd - (roughness + 1)^2 / 8

// Trowbridge-Reitz GGX normal distribution function.
// Halfway vector (H) aligned microfacets approximation. 
// Takes roughness into account.
// a -> roughness
// a^2 / PI((N dot H)^2 * (a^2 - 1) + 1)^2 
float DistributionGGX(float3 N, float3 H, float Roughness)
{
	float a2 = Roughness * Roughness;
	float NdotHSq = pow(max(dot(N, H), 0.0f), 2);
	
	return a2 / (PI * pow((NdotHSq * (a2 - 1.0f) + 1), 2));
}

// SchlickGGX - microfacet overshading geometry function.
// Microfacet is more overshaded when the roughness increases.
// Takes roughness as follows: (roughness + 1.0)^2 / 8.0
// N dot V / (N dot V)(1 - Kd) + Kd
float GeometrySchlickGGX(float NdotV, float Roughness)
{
	float Kd = pow((Roughness + 1.0f), 2) / 8.0f;

	return NdotV / (NdotV * (1.0f - Kd) + Kd);
}

// SchlickGGX - microfacet overshading geometry function.
// Microfacet is more overshaded when the roughness increases.
// Takes roughness as follows: (roughness)^2 / 2.0
// N dot V / (N dot V)(1 - Kd) + Kd
float GeometrySchlickGGX_IBL(float NdotV, float Roughness)
{
	float Kd = pow((Roughness), 2) / 2.0f;

	return NdotV / (NdotV * (1 - Kd) + Kd);
}

// Returns GeometrySchlickGGX(N, V, Roughness) * GeometrySchlickGGX(N, L, Roughness)
float GeometrySmith(float NdotV, float NdotL, float Roughness)
{
	float ggx1 = GeometrySchlickGGX(NdotV, Roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, Roughness);
	
	return ggx1 * ggx2;
}

// Returns GeometrySchlickGGX(N, V, Roughness) * GeometrySchlickGGX(N, L, Roughness) for IBL.
float GeometrySmith_IBL(float NdotV, float NdotL, float Roughness)
{
	float ggx1 = GeometrySchlickGGX_IBL(NdotV, Roughness);
	float ggx2 = GeometrySchlickGGX_IBL(NdotL, Roughness);
	
	return ggx1 * ggx2;
}


#endif // PBR_HLSLI

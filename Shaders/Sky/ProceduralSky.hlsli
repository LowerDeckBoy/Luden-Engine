#ifndef PROCEDURAL_SKY_HLSLI
#define PROCEDURAL_SKY_HLSLI

#include "../Common/Common.hlsli"

// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/s2016-pbs-frostbite-sky-clouds-new.pdf
// https://www.researchgate.net/publication/224688956_Generating_and_Rendering_Procedural_Clouds_in_Real_Time_on_Programmable_3D_Graphics_Hardware
// https://en.wikipedia.org/wiki/Rayleigh_scattering
// https://en.wikipedia.org/wiki/Mie_scattering
// https://allenliuzihao.github.io/IS-DDGI/
// https://developer.nvidia.com/blog/an-engineers-guide-to-integrating-ddgi/
// https://www.jcgt.org/published/0008/02/01/paper-lowres.pdf
// https://github.com/NVIDIAGameWorks/RTXGI-DDGI

// https://sebh.github.io/publications/egsr2020.pdf
// https://github.com/MatejSakmary/atmosphere-bac

// Scattering (x10^-6*m^-1):
// - Rayleigh	= 5.802, 13.558, 33.1
// - Mie		= 3.996, 3.996,  3.996
// - Ozone		= 0,	 0,		 0
// Absorption (x10^-6*m^-1):
// - Rayleigh	= 0,	 0,		 0
// - Mie		= 4.40,  4.40,   4.40
// - Ozone		= 0.650, 1.881,  0.085

// c - camera view position
// v - view direction
// p - intersection surface point

//static const float3 RayleighCoefficients	= float3(5.802f, 13.558f, 33.100f) * 1e-6f;
//static const float3 MieCoefficients			= float3(3.996f, 3.996f, 3.996f) * 1e-6f;

static const float3 RayleighScattering		= float3(5.802f, 13.558f, 33.100f) * 1e-6f;
static const float3 MieScattering			= float3(3.996f, 3.996f, 3.996f) * 1e-6f;
static const float3 OzoneScattering			= float3(0.0f, 0.0f, 0.0f);

static const float3 RayleighAbsoprtion		= float3(0.0f, 0.0f, 0.0f);
static const float3 MieAbsoprtion			= float3(4.40, 4.40, 4.40) * 1e-6f;
static const float3 OzoneAbsoprtion			= float3(0.650f, 1.881f, 0.085f) * 1e-6f;

static const float3 Nitrogen				= float3(0.650f, 1.881f, 0.085f) * 1e-6f;
static const float3 NitrogenExtinction		= float3(0.000650f, 0.001881f, 0.000085f);

static const float3 GroundFactor = float3(0.3f, 0.3f, 0.3f);

static const float IsotropicPhaseFactor = 1.0f / (4.0f * PI);

static const float AtmosphereHeight = 6371.0f;
static const float RayleighHeight;
static const float MieHeight;

static const float3 UpVector = float3(0.0f, 1.0f, 0.0f);

//static const float  EarthRadius			= 6360000.0f; // 6360km
static const float  EarthRadius			= 6360.0f;	// 6360km
static const float  AtmosphereRadius	= 6420.0f;	// 6420km
static const float3 EarthCenter			= float3(0, -EarthRadius, 0); // Or 0, 0, 0 for now

static const float AirIOR = 1.0003f;

// cos theta
float GetRayleighPhase(float VdotL)
{
	return (3.0f * (1.0f + VdotL * VdotL)) / (16.0f * PI);
}

float GetMiePhase(float VdotL, float G = 0.8f)
{
	const float g2 = G * G;
	
	return (3.0f / (8.0f * PI)) * ((((1.0f - g2) * (1.0f + VdotL * VdotL)) / pow(((2.0f + g2) * (1.0f + g2 - (2.0f * G * VdotL))), 1.5f)));
}

float GetRayleighHeightFactor(float Height)
{
	return exp(-Height * (1.0f / 8.0f));
}

float GetMieHeightFactor(float Height)
{
	return exp(-Height * (1.0f / 1.2f));
}

float GetOzoneHeightFactor(float Height)
{
	return max(0.0f, 1.0f - abs(Height - 25.0f) / 15.0f);
}

#endif // PROCEDURAL_SKY_HLSLI
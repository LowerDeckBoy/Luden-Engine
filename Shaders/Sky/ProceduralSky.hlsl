#ifndef PROCEDURAL_SKY_HLSL
#define PROCEDURAL_SKY_HLSL

#include "../Common/Common.hlsli"
#include "ProceduralSky_RS.hlsli"

struct SkyConstants
{
	float4x4 InversedViewProjection;
};

struct SkyParameters
{
	float3	SkyColor;
	float	RayleighCoefficient;
	float3	SunColor;
	float	MieCoefficient;
	float3	SunPosition;
	float	Turbidity;
	float3	CameraPosititon;
	float	Luminance;
	float	MieDirectionalG;
	uint	VertexBufferIndex;
};

ConstantBuffer<SkyParameters> Parameters : register(b0);
ConstantBuffer<SkyConstants> Constants : register(b1);

struct VS_OUTPUT
{
	float2 TexCoord			: TEXCOORD;
	float4 Position			: SV_POSITION;
	float3 ViewDirection	: VIEW_DIR;
};

//static const float4 FullScreenVertsPos[3] = { float4(-1.0f, 1.0f, 1.0f, 1.0f), float4(3.0f, 1.0f, 1.0f, 1.0f), float4(-1.0f, -3.0f, 1.0f, 1.0f) };
//static const float2 FullScreenVertsUVs[3] = { float2(0.0f, 0.0f), float2(2.0f, 0.0f), float2(0.0f, 2.0f) };

static const float e = 2.71828182845904523536028747135266249775724709369995957;
// wavelength of used primaries, according to preetham
static const float3 lambda = float3(680E-9, 550E-9, 450E-9);
// this pre-calcuation replaces older TotalRayleigh(float3 lambda) function:
// (8.0 * pow(pi, 3.0) * pow(pow(n, 2.0) - 1.0, 2.0) * (6.0 + 3.0 * pn)) / (3.0 * N * pow(lambda, float3(4.0)) * (6.0 - 7.0 * pn))
static const float3 totalRayleigh = float3(5.804542996261093E-6, 1.3562911419845635E-5, 3.0265902468824876E-5);

// mie stuff
// K coefficient for the primaries
static const float v = 4.0;
static const float3 K = float3(0.686, 0.678, 0.666);
// MieConst = pi * pow( ( 2.0 * pi ) / lambda, float3( v - 2.0 ) ) * K
static const float3 MieConst = float3(1.8399918514433978E14, 2.7798023919660528E14, 4.0790479543861094E14);

// earth shadow hack
// cutoffAngle = pi / 1.95;
static const float cutoffAngle = 1.6110731556870734;
static const float steepness = 1.5;
static const float EE = 1000.0;


// constants for atmospheric scattering
static const float pi = 3.141592653589793238462643383279502884197169;

static const float n = 1.0003; // refractive index of air
static const float N = 2.545E25; // number of molecules per unit volume for air at
								// 288.15K and 1013mb (sea level -45 celsius)

// optical length at zenith for molecules
static const float rayleighZenithLength = 8.4E3;
static const float mieZenithLength = 1.25E3;
static const float3 up = float3(0.0f, 1.0f, 0.0f);
// 66 arc seconds -> degrees, and the cosine of that
static const float sunAngularDiameterCos = 0.999956676946448443553574619906976478926848692873900859324;

// 3.0 / ( 16.0 * pi )
static const float THREE_OVER_SIXTEENPI = 0.05968310365946075;
// 1.0 / ( 4.0 * pi )
static const float ONE_OVER_FOURPI = 0.07957747154594767;


// TODO:
static const float3 RayleightCoeffiecients	= float3(5.802f, 13.558f, 33.100f) * 1e-6f;
static const float3 MieCoeffiecients		= float3(3.996f, 3.996f, 3.996f) * 1e-6f;
static const float3 MieAbsorbtion			= float3(4.40f, 4.40f, 4.40f) * 1e-6f;
static const float3 MieFactor				= MieCoeffiecients + MieAbsorbtion;
static const float3 OzoneCoeffiecients		= float3(0.650f, 1.881f, 0.085f) * 1e-6f;
static const float AtmosphereHeight			= 100000.0f;
static const float RayleighHeight			= (AtmosphereHeight * 0.08f);
static const float MieHeight				= (AtmosphereHeight * 0.012f);

static const float3 UpVector				= float3(0.0f, 1.0f, 0.0f);

static const float EarthRadius				= 6371000.0f;
static const float AtmosphereRadius			= 6420.0f;
static const float3 EarthCenter				= float3(0, -EarthRadius, 0);

float GetRayleighPhase(float3 V, float3 L)
{
	const float VdotL = dot(V, L) * 0.5f + 0.5f;
	//return (3.0f / (16.0f * PI)) * (1.0f + VdotL * VdotL);
	return (3.0f) * (1.0f + VdotL * VdotL) / (16.0f * PI);
}

float GetMiePhase(float3 V, float3 L, float G = 0.85f)
{
	const float VdotL = dot(V, L) * 0.5f + 0.5f;
	G = min(G, 0.9381);
	float k = 1.55 * G - 0.55 * G * G * G;
	float kcosth = k * VdotL;
	return (1 - k * k) / ((4 * PI) * (1 - kcosth) * (1 - kcosth));
}

float GetSunIntensity(float VdotL);

float GetDensityRayleigh(float Height)
{
	return exp(-max(0.0f, Height / RayleighHeight));
}

float GetDensityMie(float Height)
{
	return exp(-max(0.0f, Height / MieHeight));
}

float GetDensityOzone(float Height)
{
	// The ozone layer is represented as a tent function with a width of 30km, centered around an altitude of 25km.
	return max(0.0f, 1.0f - abs(Height - 25000.0f) / 15000.0f);
}

float3 GetAtmosphereDensity(float Height)
{
	return float3(GetDensityRayleigh(Height), GetDensityMie(Height), GetDensityOzone(Height));
}

float GetAtmosphereHeight(float3 positionWS)
{
	return distance(positionWS, EarthCenter) - EarthRadius;
} 

// Calculate a luminance transmittance value from optical depth.
float3 Absorb(float3 opticalDepth)
{
	// Note that Mie results in slightly more light absorption than scattering, about 10%
	return exp(-(opticalDepth.x * RayleightCoeffiecients + opticalDepth.y * MieCoeffiecients * 1.1 + opticalDepth.z * OzoneCoeffiecients) * 1);
}

float2 GetSphereIntersection(float3 RayOrigin, float3 RayDirection, float3 SphereCenter, float SphereRadius)
{
	//RayOrigin -= SphereCenter; // Translate ray origin to sphere-centered coordinates
	//
	//// Calculate the intersection of the ray with the sphere
	//float a = dot(RayDirection, RayDirection);
	//float b = 2.0 * dot(RayOrigin, RayDirection);
	//float c = dot(RayOrigin, RayOrigin) - SphereRadius * SphereRadius;
	//
	//float discriminant = b * b - 4.0 * a * c;
	//if (discriminant < 0.0)
	//{
	//	return float2(-1.0, -1.0); // No intersection
	//}
	//
	//float t0 = (-b - sqrt(discriminant)) / (2.0 * a);
	//float t1 = (-b + sqrt(discriminant)) / (2.0 * a);
	//
	//return float2(t0, t1);
	
	RayOrigin -= SphereCenter;
	float a = dot(RayDirection, RayDirection);
	float b = 2.0 * dot(RayOrigin, RayDirection);
	float c = dot(RayOrigin, RayOrigin) - (SphereRadius * SphereRadius);
	float d = b * b - 4 * a * c;
	if (d < 0)
	{
		return -1;
	}
	else
	{
		d = sqrt(d);
		return float2(-b - d, -b + d) / (2 * a);
	}
} 

float2 PlanetIntersection(float3 rayStart, float3 rayDir)
{
	return GetSphereIntersection(rayStart, rayDir, EarthCenter, EarthRadius);
}

float2 AtmosphereIntersection(float3 rayStart, float3 rayDir)
{
	return GetSphereIntersection(rayStart, rayDir, EarthCenter, EarthRadius + AtmosphereHeight);
}

// Optical depth is a unitless measurement of the amount of absorption of a participating medium (such as the atmosphere).
// This function calculates just that for our three atmospheric elements:
// R: Rayleigh
// G: Mie
// B: Ozone
// If you find the term "optical depth" confusing, you can think of it as "how much density was found along the ray in total".
float3 IntegrateOpticalDepth(float3 rayStart, float3 rayDir)
{
	float2 intersection = AtmosphereIntersection(rayStart, rayDir);
	float rayLength = intersection.y;

	int sampleCount = 8;
	float stepSize = rayLength / sampleCount;
	
	float3 opticalDepth = 0;

	for (int i = 0; i < sampleCount; i++)
	{
		float3 localPosition = rayStart + rayDir * (i + 0.5) * stepSize;
		float localHeight	 = GetAtmosphereHeight(localPosition);
		float3 localDensity  = GetAtmosphereDensity(localHeight);

		opticalDepth += localDensity * stepSize;
	}

	return opticalDepth;
}

float sunIntensity(float zenithAngleCos)
{
	zenithAngleCos = clamp(zenithAngleCos, -1.0, 1.0);
	return 1000.0f * max(0.0, 1.0 - pow(e, -((cutoffAngle - acos(zenithAngleCos)) / steepness)));
}

float3 totalMie(float T)
{
	float c = (0.2 * T) * 10E-18;
	return 0.434 * c * MieConst;
}

float rayleighPhase(float cosTheta)
{
	return THREE_OVER_SIXTEENPI * (1.0 + cosTheta * cosTheta);
}

float hgPhase(float cosTheta, float g)
{
	float g2 = pow(g, 2.0);
	float inverse = 1.0 / pow(abs(1.0 - 2.0 * g * cosTheta + g2), 1.5);
	return ONE_OVER_FOURPI * ((1.0 - g2) * inverse);
}

float3 GetSkyWorldPosition(float4 clipPosition)
{
	float4 world = mul(clipPosition, Constants.InversedViewProjection);
	world /= world.w;
	return world.xyz;
}

float3 GetSkyViewDirection(float2 Position)
{
	float3 near = GetSkyWorldPosition(float4(Position, -0.0, 1.0));
	float3 far  = GetSkyWorldPosition(float4(Position, +1.0, 1.0));
	
	return normalize(far - near);
}

float4 GetProceduralSky(float3 WorldPosition)
{
	//float x = pin.TexCoord.x * 2.0f - 1.0f;
	////float y = (1.0f - pin.TexCoord.y) * 2.0f - 1.0f;
	//float y = (pin.TexCoord.y) * 2.0f - 1.0f;
	//float z = 1.0f;
	//float4 clip = float4(x, y, z, 1.0f);
	//float4 worldPos = mul(clip, Constants.InversedViewProjection);
	//worldPos /= worldPos.w;
	//float3 worldPosition = worldPos.xyz;
	//float3 worldPosition = GetSkyWorldPosition(clip);
	float3 worldPosition = WorldPosition;

	float rayleigh = Parameters.RayleighCoefficient;
	float mieCoefficient = Parameters.MieCoefficient;
	float turbidity = Parameters.Turbidity;
	//float luminance = Parameters.Luminance;
	float luminance = 1.2f;
	float mieDirectionalG = Parameters.MieDirectionalG;
	//float3 cameraPos = float3(0.0f, 0.0f, 0.0f);
	const float3 centerPosition = float3(0.0f, 0.0f, 0.0f);

	float3 sunDirection = normalize(Parameters.SunPosition);
	float3 viewDirection = normalize(worldPosition - centerPosition);

	float vSunE = sunIntensity(dot(sunDirection, up));

	float vSunfade = 1.0f - clamp(1.0f - exp(sunDirection.y), 0.0f, 1.0f);

	float rayleighCoefficient = rayleigh - (1.0f * (1.0f - vSunfade));

	// extinction (absorbtion + out scattering)
	// rayleigh coefficients
	float3 betaRayleigh = RayleightCoeffiecients * rayleighCoefficient;

	// mie coefficients
	//float3 betaMie = totalMie(turbidity) * mieCoefficient;
	float3 betaMie = GetMiePhase(viewDirection, sunDirection, turbidity) * MieCoeffiecients * mieCoefficient;

	// optical length
	// cutoff angle at 90 to avoid singularity in next formula.
	float zenithAngle = acos(max(0.0, dot(up, viewDirection)));
	float inverse = 1.0 / (cos(zenithAngle) + 0.15 * pow(abs(93.885 - ((zenithAngle * 180.0) / pi)), -1.253));
	float sR = rayleighZenithLength * inverse;
	float sM = mieZenithLength * inverse;

	// combined extinction factor
	float3 Fex = exp(-(betaRayleigh * sR + betaMie * sM));

	// in scattering; cosTheta
	float VdotL = dot(viewDirection, sunDirection);

	//float rayleighPhase = rayleighPhase(cosTheta * 0.5 + 0.5);
	//float rayleighPhase = GetRayleighPhase(cosTheta * 0.5 + 0.5);
	float rayleighPhase = GetRayleighPhase(viewDirection, sunDirection);
	float3 betaRTheta = betaRayleigh * rayleighPhase;

	float miePhase = hgPhase(VdotL, mieDirectionalG);
	float3 betaMTheta = betaMie * miePhase;

	float3 Lin = pow(abs(vSunE * ((betaRTheta + betaMTheta) / (betaRayleigh + betaMie)) * (1.0 - Fex)), float3(1.5, 1.5, 1.5));
	Lin *= lerp(float3(1.0, 1.0, 1.0), pow(vSunE * ((betaRTheta + betaMTheta) / (betaRayleigh + betaMie)) * Fex, float3(1.0 / 2.0, 1.0 / 2.0, 1.0 / 2.0)), clamp(pow(1.0 - dot(up, sunDirection), 5.0), 0.0, 1.0));

	// nightsky
	//float3 direction = normalize(worldPosition - centerPosition);
	//float3 direction = viewDirection;
	//float theta = acos(direction.y); // elevation --> y-axis, [-pi/2, pi/2]',
	//float phi = atan2(direction.z, direction.x); // azimuth --> x-axis [-pi/2, pi/2]',
	//float2 uv = float2(phi, theta) / float2(2.0 * pi, pi) + float2(0.5, 0.0);
	//float3 L0 = float3(0.1f, 0.1f, 0.1f) * Fex;
	float3 L0 = float3(Parameters.SkyColor) * Fex;

	// composition + solar disc
	float sundisk = smoothstep(sunAngularDiameterCos, sunAngularDiameterCos + 0.00002, VdotL);
	L0 += (vSunE * 119000.0 * Fex) * sundisk;

	float3 texColor = (Lin + L0) * 0.04 + float3(0.0, 0.0003, 0.00075);

	float3 color = (log2(2.0f / pow(luminance, 4.0f))) * texColor;

	float3 output = pow(abs(color), (1.0 / (1.2 + (1.2 * vSunfade))));

	return float4(output, 1.0);
}

float4 GetProceduralSky_TEST(float3 WorldPosition)
{
	const float3 centerPosition = float3(0.0f, 0.0f, 0.0f);

	float3 lighDir = -Parameters.SunPosition;
	float3 viewDirection = normalize(WorldPosition - centerPosition);

	//float3 rayStart = viewDirection;
	float3 rayStart = Parameters.CameraPosititon;
	//float3 rayDir = -Parameters.SunPosition;
	float3 rayDir = -viewDirection;
	
	float rayLength = 1000000.0f;
	float2 intersection = AtmosphereIntersection(rayStart, rayDir);
	rayLength = min(rayLength, intersection.y);
	if (intersection.x > 0)
	{
		// Advance ray to the atmosphere entry point
		rayStart += rayDir * intersection.x;
		rayLength -= intersection.x;
	}
	
	float VdotL = dot(viewDirection, lighDir);
	float phaseRayleigh = GetRayleighPhase(viewDirection, lighDir);
	float phaseMie = GetMiePhase(viewDirection, lighDir);
	
	float rayHeight = GetAtmosphereHeight(rayStart);
	
	float3 rayleigh = 0;
	float3 mie = 0;
	float3 opticalDepth = 0;
	
	float prevRayTime = 0;
	float sampleDistributionExponent = 1 + saturate(1 - rayHeight / AtmosphereHeight) * 8; // Slightly arbitrary max exponent of 9
	for (int i = 0; i < 64; i++)
	{
		//float rayTime = pow((float) i / 64, sampleDistributionExponent) * 1000000.0f;
		float rayTime = pow((float) i / 64, sampleDistributionExponent) * rayLength;
		// Because we are distributing the samples exponentially, we have to calculate the step size per sample.
		float stepSize = (rayTime - prevRayTime);
		
		float3 localPosition = rayStart + rayDir * rayTime;
		float localHeight = GetAtmosphereHeight(localPosition);
		float3 localDensity = GetAtmosphereDensity(localHeight);
		
		
		opticalDepth += localDensity * stepSize;
		// The atmospheric transmittance from rayStart to localPosition
		float3 viewTransmittance = Absorb(opticalDepth);
		
		float3 opticalDepthlight = IntegrateOpticalDepth(localPosition, lighDir);
		// The atmospheric transmittance of light reaching localPosition
		float3 lightTransmittance = Absorb(opticalDepthlight);
		
		rayleigh	+= viewTransmittance * lightTransmittance * phaseRayleigh	* localDensity.x * stepSize;
		mie			+= viewTransmittance * lightTransmittance * phaseMie		* localDensity.y * stepSize;

		prevRayTime = rayTime;
	}
	
	float3 output = (rayleigh * RayleightCoeffiecients + mie * MieCoeffiecients) * Parameters.SkyColor * 1.0f;
	
	return float4(output, 1.0f);
}

struct VS_INPUT
{
	float2 Position : POSITION;
};

[RootSignature(PROCEDURAL_SKY_RS)]
VS_OUTPUT VSMain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	StructuredBuffer<VS_INPUT> vertices = ResourceDescriptorHeap[Parameters.VertexBufferIndex];
	
	output.Position = float4(vertices[VertexID].Position, 1.0f, 1.0f);
	output.TexCoord = output.Position.xy;
	output.ViewDirection = GetSkyViewDirection(output.TexCoord.xy);
	//output.Position = FullScreenVertsPos[VertexID];
	//output.TexCoord = FullScreenVertsUVs[VertexID];

	//output.ViewDirection = GetSkyViewDirection(FullScreenVertsPos[VertexID].xy);
	
	return output;
}

// https://github.com/TomCrypto/final-project/blob/master/doc/Papers/A%20Practical%20Analytic%20Model%20for%20Daylight.pdf
// https://cpp-rendering.io/sky-and-atmosphere-rendering/
// https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/master/src/DX12/shaders/SkyDomeProc.hlsl
// https://github.com/Zydak/Vulkan-Path-Tracer/blob/main/PathTracer/Shaders/RTCommon.slang
// https://github.com/simco50/D3D12_Research/blob/master/Resources/Shaders/External/Atmosphere.hlsli
float4 PSMain(VS_OUTPUT pin) : SV_TARGET
{
	return GetProceduralSky(pin.ViewDirection);
	return GetProceduralSky_TEST(pin.ViewDirection);
}

/*
struct VS_INPUT
{
	float2 Position : POSITION;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 ViewDirection : VIEW_DIR;
};

[RootSignature(SKY_RS)]
VS_OUTPUT VSMain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	StructuredBuffer<VS_INPUT> vertices = ResourceDescriptorHeap[Parameters.VertexBufferIndex];
	
	float2 position = vertices[VertexID].Position;
	// TODO: negate Y on C++ side.
	position.y *= -1.0f;
	
	float4 rayStart = mul(float4(position, -1.0, 1.0f), Constants.World);
	float4 rayEnd = mul(float4(position, +1.0, 1.0f), Constants.World);
	
	rayStart = rayStart / rayStart.w;
	rayEnd = rayEnd / rayEnd.w;
	
	output.ViewDirection = normalize(rayEnd.xyz - rayStart.xyz);
	//output.ViewDirection.y = abs(output.ViewDirection.y);
	
	return output;
}

float4 PSMain(VS_OUTPUT pin) : SV_TARGET
{
	float sunSize = Parameters.SunSize;
	float sunBloom = Parameters.SunBloom;
	float size2 = sunSize * sunSize;
	float3 sunDirection = normalize(Parameters.SunPosition);

	float distance = 2.0f - (1.0f - dot(normalize(pin.ViewDirection), sunDirection));
	float sun = exp(-distance / sunBloom / size2) + step(distance, size2);
	float sun2 = min(sun * sun, 1.0);
	float3 color = Parameters.SkyColor.rgb + (sun * Parameters.SunColor);
	
	return float4(color, 1.0f);
	return float4(Parameters.SkyColor, 1.0f);
}
*/

#endif // PROCEDURAL_SKY_HLSL

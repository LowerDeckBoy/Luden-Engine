#ifndef PROCEDURAL_SKY_HLSL
#define PROCEDURAL_SKY_HLSL

//#include "../Common/Common.hlsli"
#include "ProceduralSky_RS.hlsli"

struct SkyConstants
{
	float4x4 InversedViewProjection;
};

struct SkyParameters
{
	float3	SkyColor;
	float	Rayleigh;
	float3	SunColor;
	float	MieCoefficient;
	float3	SunPosition;
	float	Turbidity;
	float3	CameraPosititon;
	float	Luminance;
	float	MieDirectionalG;
};

ConstantBuffer<SkyParameters> Parameters : register(b0);
ConstantBuffer<SkyConstants> Constants : register(b1);

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
*/

struct VS_OUTPUT
{
	float2 TexCoord			: TEXCOORD;
	float4 Position			: SV_POSITION;
	float3 ViewDirection	: VIEW_DIR;
};

static const float4 FullScreenVertsPos[3] = { float4(-1.0f, 1.0f, 1.0f, 1.0f), float4(3.0f, 1.0f, 1.0f, 1.0f), float4(-1.0f, -3.0f, 1.0f, 1.0f) };
static const float2 FullScreenVertsUVs[3] = { float2(0.0f, 0.0f), float2(2.0f, 0.0f), float2(0.0f, 2.0f) };

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


/////////////////

//static const float3 cameraPos = float3(0.0, 0.0, 0.0);

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
	return THREE_OVER_SIXTEENPI * (1.0 + pow(cosTheta, 2.0));
}

float hgPhase(float cosTheta, float g)
{
	float g2 = pow(g, 2.0);
	float inverse = 1.0 / pow(abs(1.0 - 2.0 * g * cosTheta + g2), 1.5);
	return ONE_OVER_FOURPI * ((1.0 - g2) * inverse);
}

[RootSignature(PROCEDURAL_SKY_RS)]
VS_OUTPUT VSMain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	output.Position = FullScreenVertsPos[VertexID];
	output.TexCoord = FullScreenVertsUVs[VertexID];
	
	return output;
}

struct VERTEX
{
	float2 TexCoord : TEXCOORD;
};

// https://github.com/TomCrypto/final-project/blob/master/doc/Papers/A%20Practical%20Analytic%20Model%20for%20Daylight.pdf
// https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/master/src/DX12/shaders/SkyDomeProc.hlsl
float4 PSMain(VS_OUTPUT pin) : SV_TARGET
{
	float x = pin.TexCoord.x * 2.0f - 1.0f;
	float y = (1.0f - pin.TexCoord.y) * 2.0f - 1.0f;
	float z = 1.0f;
	float4 clip = float4(x, y, z, 1);
	float3 worldPosition = mul(clip, Constants.InversedViewProjection).xyz;

	//return float4(vWorldPosition, 1.0f);

	float rayleigh = Parameters.Rayleigh;
	float mieCoefficient = Parameters.MieCoefficient;
	float turbidity = Parameters.Turbidity;
	float luminance = Parameters.Luminance;
	float mieDirectionalG = Parameters.MieDirectionalG;
	//float3 cameraPos = normalize(Parameters.CameraPosititon);
	float3 cameraPos = float3(0.0f, 0.0f, 0.0f);

	float3 sunDirection = normalize(Parameters.SunPosition);

	float vSunE = sunIntensity(dot(sunDirection, up));

	float vSunfade = 1.0 - clamp(1.0 - exp(sunDirection.y), 0.0, 1.0);

	float rayleighCoefficient = rayleigh - (1.0 * (1.0 - vSunfade));

    // extinction (absorbtion + out scattering)
    // rayleigh coefficients
	float3 vBetaR = totalRayleigh * rayleighCoefficient;

    // mie coefficients
	float3 vBetaM = totalMie(turbidity) * mieCoefficient;

    // optical length
    // cutoff angle at 90 to avoid singularity in next formula.
	float zenithAngle = acos(max(0.0, dot(up, normalize(worldPosition - cameraPos))));
	float inverse = 1.0 / (cos(zenithAngle) + 0.15 * pow(abs(93.885 - ((zenithAngle * 180.0) / pi)), -1.253));
	float sR = rayleighZenithLength * inverse;
	float sM = mieZenithLength * inverse;

    // combined extinction factor
	float3 Fex = exp(-(vBetaR * sR + vBetaM * sM));

    // in scattering
	float cosTheta = dot(normalize(worldPosition - cameraPos), sunDirection);

	float rPhase = rayleighPhase(cosTheta * 0.5 + 0.5);
	float3 betaRTheta = vBetaR * rPhase;

	float mPhase = hgPhase(cosTheta, mieDirectionalG);
	float3 betaMTheta = vBetaM * mPhase;

	float3 Lin = pow(abs(vSunE * ((betaRTheta + betaMTheta) / (vBetaR + vBetaM)) * (1.0 - Fex)), float3(1.5, 1.5, 1.5));
	Lin *= lerp(float3(1.0, 1.0, 1.0), pow(vSunE * ((betaRTheta + betaMTheta) / (vBetaR + vBetaM)) * Fex, float3(1.0 / 2.0, 1.0 / 2.0, 1.0 / 2.0)), clamp(pow(1.0 - dot(up, sunDirection), 5.0), 0.0, 1.0));

    // nightsky
	float3 direction = normalize(worldPosition - cameraPos);
	float theta = acos(direction.y); // elevation --> y-axis, [-pi/2, pi/2]',
	float phi = atan2(direction.z, direction.x); // azimuth --> x-axis [-pi/2, pi/2]',
	float2 uv = float2(phi, theta) / float2(2.0 * pi, pi) + float2(0.5, 0.0);
	float3 L0 = float3(0.1, 0.1, 0.1) * Fex;

    // composition + solar disc
	float sundisk = smoothstep(sunAngularDiameterCos, sunAngularDiameterCos + 0.00002, cosTheta);
	L0 += (vSunE * 19000.0 * Fex) * sundisk;

	float3 texColor = (Lin + L0) * 0.04 + float3(0.0, 0.0003, 0.00075);

	//no tonemapped
	float3 color = (log2(2.0 / pow(luminance, 4.0))) * texColor;

	float3 output = pow(abs(color), (1.0 / (1.2 + (1.2 * vSunfade))));

	return float4(output, 1.0);
}

/*
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

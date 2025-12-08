#ifndef SKYBOX_HLSL
#define SKYBOX_HLSL

#include "../Common/Common.hlsli"
#include "Sky_RS.hlsli"
#include "ProceduralSky.hlsli"

struct SkyConstants
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
};

struct SkyParameters
{
	float4 SkyColor;
	float4 SunColor;
	float3 CameraPosititon;
	float padding;
	float3 LightDirection;
	float padding2;
};

static const float3 Vertices[8] =
{
	float3(-1.0f, +1.0f, +1.0f),
	float3(+1.0f, +1.0f, +1.0f),
	float3(+1.0f, -1.0f, +1.0f),
	float3(-1.0f, -1.0f, +1.0f),
	float3(+1.0f, +1.0f, -1.0f),
	float3(-1.0f, +1.0f, -1.0f),
	float3(-1.0f, -1.0f, -1.0f),
	float3(+1.0f, -1.0f, -1.0f)
};

float2 DirectionToEquirectUV(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(-v.y));
	uv /= float2(-TwoPI, PI);
	uv += float2(0.5, 0.5);
	return uv;
}

float2 SampleSphericalMap(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(-v.y));
	uv *= float2(0.1591f, 0.3183f);
	uv += 0.5f;
	return uv;
}

float GetSunPosition(float3 V, float3 V2)
{
	return acos(dot(normalize(V), normalize(V2)));
}

ConstantBuffer<SkyParameters> Parameters : register(b0);
ConstantBuffer<SkyConstants>  Constants : register(b1);

float3 GetSimpleSkyColor(float3 V, float3 L, float2 UV)
{
	float3 dayColor = float3(0.043, 0.961, 0.761);
	float3 horizonColor = float3(0.949, 0.49, 0);
	
	float lightMask = smoothstep(-0.1f, 0.1f, L.y);
	
	float horizonMask = pow(smoothstep(.40, .52, UV.y), 3);
	
	float horizonActivation = pow(smoothstep(.25, 0, Parameters.LightDirection.y), 3) * smoothstep(-.2, .15, Parameters.LightDirection.y);
	
	float3 output = lerp(dayColor, horizonColor, horizonMask);
	output = lerp(output, horizonColor, horizonMask * horizonActivation);
	
	return output;
}

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
	float3 ViewDirection : VIEW_DIR;
};

[RootSignature(SKY_RS)]
VS_OUTPUT VSMain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	float3 position		= mul(float4(Vertices[VertexID], 1.0f), Constants.View).xyz;
	output.Position		= mul(float4(position, 1.0f), Constants.Projection);
	output.Position.z	= output.Position.w;
	
	output.TexCoord = Vertices[VertexID];

	//const float2 pos = output.Position.xy;
	const float2 pos = Vertices[VertexID].xy;
	float4 rayStart = mul(float4(pos, -1.0, 1.0f), Constants.World);
	float4 rayEnd	= mul(float4(pos, +1.0, 1.0f), Constants.World);
	
	rayStart = rayStart / rayStart.w;
	rayEnd = rayEnd / rayEnd.w;
	
	output.ViewDirection = normalize(rayEnd.xyz - rayStart.xyz);
	//output.ViewDirection.y = abs(output.ViewDirection.y);
	
	//output.ViewDirection
	
	return output;
}

// https://flareonz44.github.io/procedural-skybox-shader
// https://github.com/shff/opengl_sky
// https://github.com/podgorskiy/ProceduralSky_bgfx/blob/master/sources/fs_proceduralsky_sky.sc
float4 PSMain(VS_OUTPUT pin) : SV_TARGET
{
	//if (Parameters.LightDirection.y < 0.0f)
	//{
	//	discard;
	//}
	
	float sunSize = 0.02f;
	float sunBloom = 3.0f;
	float size2 = sunSize * sunSize;

	float3 UV = pin.TexCoord;
	float3 V = normalize(pin.TexCoord - Parameters.CameraPosititon);
	//float2 uv = DirectionToEquirectUV(V);
	float2 uv = SampleSphericalMap(V);
	float3 L = Parameters.LightDirection;
	
	//float4 rayStart = mul(float4(pin.Position.xy, -1.0, 1.0f), Constants.World);
	//float4 rayEnd	= mul(float4(pin.Position.xy, +1.0, 1.0f), Constants.World);
	//
	//rayStart = rayStart / rayStart.w;
	//rayEnd = rayEnd / rayEnd.w;
	//
	//float3 viewDirection = normalize(rayEnd.xyz - rayStart.xyz);
	//viewDirection.y = abs(viewDirection.y);
	
	float3 output = float3(0.0f, 0.0f, 0.0f);

	float3 sunDirection = normalize(-Parameters.LightDirection);
	//float distance = 2.0f - (1.0f - dot(normalize(Parameters.CameraPosititon), sunDirection));//-1.0f;
	//float distance = 2.0f - (1.0f - dot(V, sunDirection)); //-1.0f;
	float distance = 2.0f - (1.0f - dot(normalize(pin.ViewDirection), sunDirection)); //-1.0f;
	//float sun = exp(-distance / u_parameters.y / size2) + step(distance, size2);
	float sun = exp(-distance / sunBloom / size2) + step(distance, size2);
	//float sun = acos(normalize(dot(Parameters.SkyColor.rgb, V)));
	float sun2 = min(sun * sun, 1.0);
	float3 color = Parameters.SkyColor.rgb + sun;

	
	//float fCos = normalize(dot(Parameters.LightDirection.xyz, UV));

	//GetSunPosition()
	
	return float4(color, 1.0f);
	return float4(GetSimpleSkyColor(V, normalize(L), uv), 1.0f);


	return float4(output, 1.0f);
	//return float4(procedural, 1.0f);
}

#endif // SKYBOX_HLSL

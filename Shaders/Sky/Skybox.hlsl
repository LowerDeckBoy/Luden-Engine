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

ConstantBuffer<SkyParameters> Parameters : register(b0);
ConstantBuffer<SkyConstants>  Constants : register(b1);

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

[RootSignature(SKY_RS)]
VS_OUTPUT VSMain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	//float4x4 vp = mul(Constants.View, Constants.Projection);
	float3 position = mul(float4(Vertices[VertexID], 0.0f), Constants.View).xyz;
	output.Position = mul(float4(position, 1.0f), Constants.Projection);
	output.Position.z = output.Position.w;
	
	output.TexCoord = Vertices[VertexID];
	
	return output;
}

float4 PSMain(VS_OUTPUT pin) : SV_TARGET
{
	//float3 V = normalize(pin.TexCoord);
	float3 V = normalize(float3(pin.TexCoord.xy * 2.0f - 1.0f, 1.0f));
	float3 L = -Parameters.LightDirection;
	
	float3 procedural = GetProceduralSky(Parameters.SkyColor.rgb, Parameters.SunColor.rgb, V, L);
	
	return float4(procedural, 1.0f);
}

#endif // SKYBOX_HLSL

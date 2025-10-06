#ifndef SKYBOX_HLSL
#define SKYBOX_HLSL

#include "Sky_RS.hlsli"

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

ConstantBuffer<SkyConstants> Constants : register(b1);
ConstantBuffer<SkyParameters> Paramters : register(b0);

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

[RootSignature(SKY_RS)]
VS_OUTPUT VSMain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	float4x4 wvp = mul(Constants.World, mul(Constants.View, transpose(Constants.Projection)));
	output.Position = mul(wvp, float4(Vertices[VertexID], 1.0f)).xyww;
	output.TexCoord = Vertices[VertexID];

	return output;
}

float4 PSMain() : SV_TARGET
{
	float3 color = Paramters.SkyColor.rgb;

	return float4(color, 1.0f);
}

#endif // SKYBOX_HLSL

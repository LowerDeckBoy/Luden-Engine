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
	//float horizonMask = pow(smoothstep(DegreesToRadians(.40), DegreesToRadians(.52), UV.y), 3);
	
	float horizonActivation = pow(smoothstep(.25, 0, Parameters.LightDirection.y), 3) * smoothstep(-.2, .15, Parameters.LightDirection.y);
	
	float3 output = lerp(dayColor, horizonColor, horizonMask);
	output = lerp(output, horizonColor, horizonMask * horizonActivation);
	
	return output;
}

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
	float3 position = mul(float4(Vertices[VertexID], 1.0f), transpose(Constants.View)).xyz;
	//float3 position = mul(float4(Vertices[VertexID], 1.0f), Constants.World).xyz;
	//position = mul(float4(position, 1.0f), Constants.View).xyz;
	output.Position = mul(float4(position, 1.0f), Constants.Projection);
	output.Position.z = output.Position.w;
	
	output.TexCoord = Vertices[VertexID];
	
	return output;
}

// https://flareonz44.github.io/procedural-skybox-shader
// https://github.com/shff/opengl_sky
float4 PSMain(VS_OUTPUT pin) : SV_TARGET
{
	if (-Parameters.LightDirection.y < 0.0f)
	{
		discard;
	}

	float3 V = normalize(pin.TexCoord);
	//float3 V = normalize(float3(pin.TexCoord.xy * 2.0f - 1.0f, -1.0f));
	//float3 V = normalize(pin.TexCoord.xyz - Parameters.CameraPosititon);
	//float2 uv = DirectionToEquirectUV(V);
	float2 uv = float2(atan2(V.z, V.x) / HalfPI, (asin(V.y) + HalfPI) / PI);
	//float3 V = normalize(float3(pin.TexCoord.xy * 2.0f - 1.0f, 1.0f));
	float3 L = Parameters.LightDirection;
	
	float3 output = float3(0.0f, 0.0f, 0.0f);
	
	//float ang_sun = GetSunPosition(V, normalize(L));
	//
	//
	//if (ang_sun < .1f)
	//{
	//	output = float3(Parameters.SunColor.rgb);
	//}
	//else
	//{
	//	output = float3(Parameters.SkyColor.rgb);
	//}

	return float4(GetSimpleSkyColor(V, normalize(L), uv), 1.0f);

	return float4(output, 1.0f);
	//return float4(procedural, 1.0f);
}

#endif // SKYBOX_HLSL

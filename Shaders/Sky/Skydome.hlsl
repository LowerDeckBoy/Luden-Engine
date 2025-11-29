#ifndef SKYDOME_HLSL
#define SKYDOME_HLSL

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
	float3 SkyColor;
	float SunSize;
	float3 SunColor;
	float SunBloom;
	float3 LightDirection;
	uint VertexBufferIndex;
	float3 CameraPosititon;
};

float2 DirectionToEquirectUV(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(-v.y));
	uv /= float2(-TwoPI, PI);
	uv += float2(0.5, 0.5);
	return uv;
}

ConstantBuffer<SkyParameters> Parameters : register(b0);
ConstantBuffer<SkyConstants> Constants : register(b1);

struct VS_INPUT
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
	float3 ViewDirection : VIEW_DIR;
};

float GetSunPosition(float3 V, float3 V2)
{
	return acos(dot(normalize(V), normalize(V2)));
}

VS_INPUT LoadVertex(uint Location)
{
	StructuredBuffer<VS_INPUT> buffer = ResourceDescriptorHeap[Parameters.VertexBufferIndex];
	VS_INPUT vertex = buffer.Load(Location);
	
	return vertex;
}

[RootSignature(SKY_RS)]
VS_OUTPUT VSMain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	VS_INPUT vertex = LoadVertex(VertexID);
	
	float3 position = mul(float4(vertex.Position.xyz, 1.0f), Constants.View).xyz;
	position = mul(float4(position, 1.0f), Constants.Projection).xyz;
	output.Position = float4(position, 1.0f);
	output.Position.z = output.Position.w;
	
	output.TexCoord = vertex.TexCoord.xy;
	
	//const float2 pos = Vertex
	float4 rayStart = mul(float4((vertex.Position.xy), -1.0, 1.0f), Constants.World);
	float4 rayEnd	= mul(float4((vertex.Position.xy), +1.0, 1.0f), Constants.World);
	
	rayStart = rayStart / rayStart.w;
	rayEnd = rayEnd / rayEnd.w;
	
	output.ViewDirection = normalize(rayEnd.xyz - rayStart.xyz);
	output.ViewDirection.y = abs(output.ViewDirection.y);
	
	return output;
}

// https://github.com/podgorskiy/ProceduralSky_bgfx/blob/master/sources/fs_proceduralsky_sky.sc
float4 PSMain(VS_OUTPUT pin) : SV_TARGET
{
	//float3 V = normalize(float3(pin.TexCoord * 2.0f - 1.0f, -1.0f));
	//float3 V = normalize(float3(pin.ViewDirection.xy * 2.0f - 1.0f, 1.0f));
	
	float sunSize	= Parameters.SunSize;
	float sunBloom	= Parameters.SunBloom;
	float size2		= sunSize * sunSize;
	//float3 sunDirection = normalize(Parameters.LightDirection);
	float3 sunDirection = normalize(float3(0.0f, 1.0f, 0.0f));

	float distance = 2.0f - (1.0f - dot(normalize(pin.ViewDirection), sunDirection));
	//float distance = 2.0f - (1.0f - dot(V, sunDirection));
	float sun = exp(-distance / sunBloom / size2) + step(distance, size2);
	float sun2 = min(sun * sun, 1.0);
	float3 color = Parameters.SkyColor.rgb + (sun * Parameters.SunColor);
	
	return float4(color, 1.0f);

}

#endif // SKYDOME_HLSL

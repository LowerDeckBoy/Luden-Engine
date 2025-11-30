#ifndef PROCEDURAL_SKY_HLSL
#define PROCEDURAL_SKY_HLSL

#include "../Common/Common.hlsli"
#include "Sky_RS.hlsli"

struct SkyConstants
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
};

struct SkyParameters
{
	float3	SkyColor;
	float	SunSize;
	float3	SunColor;
	float	SunBloom;
	float3	SunPosition;
	uint	VertexBufferIndex;
	float3	CameraPosititon;
};

ConstantBuffer<SkyParameters> Parameters : register(b0);
ConstantBuffer<SkyConstants> Constants : register(b1);

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
	rayEnd	 = rayEnd / rayEnd.w;
	
	output.ViewDirection = normalize(rayEnd.xyz - rayStart.xyz);
	//output.ViewDirection.y = abs(output.ViewDirection.y);
	
	output.Position = float4(position, 1.0, 1.0f);
	
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

#endif // PROCEDURAL_SKY_HLSL

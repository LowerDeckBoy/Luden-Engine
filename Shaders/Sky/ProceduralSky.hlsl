#ifndef PROCEDURAL_SKY_HLSL
#define PROCEDURAL_SKY_HLSL

#include "../Common/Common.hlsli"
#include "Sky_RS.hlsli"

struct SkyConstants
{
	float4x4 InversedViewProjection;
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

ConstantBuffer<SkyParameters>	Parameters	: register(b0);
ConstantBuffer<SkyConstants>	Constants	: register(b1);

struct VertexInput
{
	float2 Position		: POSITION;
};

struct VertexOutput
{
	float4 Position			: SV_POSITION;
	float3 ViewDirection	: VIEW_DIRECTION;
};

// https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering
// https://tips.clip-studio.com/en-us/articles/9082
[RootSignature(SKY_RS)]
VertexOutput VSMain(uint VertexID : SV_VertexID)
{
	VertexOutput output = (VertexOutput) 0;

	StructuredBuffer<VertexInput> vertices = ResourceDescriptorHeap[Parameters.VertexBufferIndex];
	
	float2 position = vertices[VertexID].Position;
	output.Position = float4(position, 1.0, 1.0f);

	float4 rayStart = mul(float4(position, -1.0, 1.0f), Constants.InversedViewProjection);
	float4 rayEnd	= mul(float4(position, +1.0, 1.0f), Constants.InversedViewProjection);
	
	rayStart = rayStart / rayStart.w;
	rayEnd	 = rayEnd / rayEnd.w;
	
	output.ViewDirection = normalize(rayEnd.xyz - rayStart.xyz);
	//output.ViewDirection.y = abs(output.ViewDirection.y);
	
	return output;
}

// https://www.rastertek.com/tertut10.html
float4 PSMain(VertexOutput pin) : SV_TARGET
{
	float sunSize = Parameters.SunSize;
	float sunBloom = Parameters.SunBloom;
	float size2 = sunSize * sunSize;
	float3 sunDirection = normalize(Parameters.SunPosition);

	float VdotL = dot(normalize(pin.ViewDirection), sunDirection);
	float distance = 2.0f - (1.0f - VdotL);
	float sun = exp(-distance / sunBloom / size2) + step(distance, size2);
	float sun2 = min(sun * sun, 1.0f);
	
	float3 color = Parameters.SkyColor.rgb + (sun2 * Parameters.SunColor);
	
	//float horizon_activation = pow(smoothstep(.25, 0, sunDirection.y), 3) * smoothstep(-.2, .15, sunDirection.y);
	//float3 horizonColor = float3(0.949f, 0.49f, 0.0f);
	//float horizon_mask = pow(smoothstep(.40, .52, normalize(pin.Position.y)), 3);
	//float3 output = lerp(color, horizonColor, horizon_mask * horizon_activation);
	
	//float3 sunHalo =
	//	lerp(
	//		max(0.f, (1.f - max(0.f, (1.f - sunDirection.y * sunBloom)) * pin.ViewDirection.y * size2)),
	//		//max(0.f, (1.f - max(0.f, (1.f - sunDirection.y * 3.f)) * pin.ViewDirection.y * 4.f)),
	//		0.f,
	//		sunDirection.y)
	//	//* saturate(pow(saturate(0.5f * VdotL + 0.5f), (8.f - sunDirection.y * 5.f)))
	//	* saturate(pow(saturate(0.5f * VdotL + 0.5f), (distance - sunDirection.y * 5.f)))
	//	* Parameters.SunColor;
		
	return float4(color, 1.0f);
}

#endif // PROCEDURAL_SKY_HLSL

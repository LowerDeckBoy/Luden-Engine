#ifndef DEFERRED_HLSL
#define DEFERRED_HLSL

#include "Deferred_RS.hlsli"

struct ScreenQuadOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

ScreenQuadOutput VSMain(uint VertexID : SV_VertexID)
{
	ScreenQuadOutput output = (ScreenQuadOutput) 0;
	
	output.TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	output.Position = float4(output.TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
	output.Position.y *= -1.0f;
	
	return output;
}

[RootSignature(ROOT_SIG)]
float4 PSMain(ScreenQuadOutput pin) : SV_TARGET0
{

	float3 output = float3(1.0f, 1.0f, 1.0f);

	return float4(pin.TexCoord, 0.0f, 1.0f);
	return float4(output, 1.0f);
}

#endif // DEFERRED_HLSL

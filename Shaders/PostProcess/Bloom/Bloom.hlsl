#ifndef BLOOM_HLSL
#define BLOOM_HLSL

#include "../../Common/Bindless.hlsli"
#include "Common.hlsli"
#include "Bloom_RS.hlsli"
#include "../../Common/Color.hlsli"

float3 KarisAverage(float3 A, float3 B, float3 C, float3 D)
{
	float3 weightA = 1.0f / (1.0f + GetLuminance(A));
	float3 weightB = 1.0f / (1.0f + GetLuminance(B));
	float3 weightC = 1.0f / (1.0f + GetLuminance(C));
	float3 weightD = 1.0f / (1.0f + GetLuminance(D));
	
	return (A * weightA + B * weightB + C * weightC + D * weightD) / (weightA + weightB + weightC + weightD);
}

float3 GetThreshold(float3 Color)
{
	float luminance = GetLuminance(Color);
	
	float brightness = luminance;
	float knee = Constants.Threshold * Constants.ThresholdSoft;
    
	float soft = brightness - Constants.Threshold + knee;
    
	soft = clamp(soft, 0.0, 2.0 * knee);
    
	soft = soft * soft / (4.0 * knee + 0.00001);
    
	float contribution = max(soft, brightness - Constants.Threshold);
    
	contribution /= max(brightness, 0.00001);
	return float3(Color * contribution);
}

[RootSignature(BLOOM_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> output 	= GetRWTexture<float4>(Constants.MipIndex);
	const float2 textureSize 	= GetTextureSize(output);

	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}

	Texture2D<float4> emissiveTexture = GetTexture(Constants.EmissiveImageIndex);
	Texture2D<float4> hdrTexture = GetTexture(Constants.LightImageIndex);
		
	float3 emissive = emissiveTexture.Load(uint3(DispatchThreadID.xy, 0)).rgb;
	float3 color = hdrTexture.Load(uint3(DispatchThreadID.xy, 0)).rgb;
	color += emissive;
	if (Constants.bFilterThreshold)
	{
		float falloffRange = Constants.ThresholdSoft;
		float falloffStart = Constants.Threshold - falloffRange;
		float falloffEnd = Constants.Threshold + falloffRange;
		float factor = smoothstep(falloffStart, falloffEnd, GetLuminance(color));
		color *= factor;
	}
	
	//output[DispatchThreadID.xy] = float4(color, 1.0f);
	output[DispatchThreadID.xy] = float4(emissive, 1.0f);
	//output[DispatchThreadID.xy] = float4(color + emissive, 1.0f);
}

#endif // BLOOM_HLSL

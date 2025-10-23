#ifndef SCATTERING_HLSL
#define SCATTERING_HLSL

#include "Scattering_RS.hlsli"
#include "../../Common/Common.hlsli"
#include "../../Common/Bindless.hlsli"

#define DISPATCH_BLOCK 8

struct Parameters
{
	uint SceneImageIndex;
	uint OutputImageIndex;
	
	float IlluminationDecay;
	float Weight;

	float3 ScreenLightPosition;
	float padding;
};

ConstantBuffer<Parameters> Constants : register(b0);

SamplerState LinearWrapSampler : register(s0);

[RootSignature(SCATTERING_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4> sceneTexture 		= GetTexture(Constants.SceneImageIndex);
	RWTexture2D<float4> outputTexture 	= GetRWTexture<float4>(Constants.OutputImageIndex);

	float2 textureSize 	= GetTextureSize(sceneTexture);
	float2 texelSize	= GetTexelSize(textureSize);
	float2 texCoord 	= (float2(DispatchThreadID.xy) + 0.5f) * texelSize;

	float2 screenLight = float2(1.0, 1.0f);
	// Calculate vector from pixel to light source in screen space.
  	//half2 deltaTexCoord = (texCoord - Constants.ScreenLightPosition.xy);
	half2 deltaTexCoord = (texCoord - screenLight.xy);
	uint NUM_SAMPLES = 64;
	float Density = 1;
	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;

	half3 color = sceneTexture.Sample(LinearWrapSampler, texCoord).rgb;

	float illuminationDecay = 1.0f;
	//float decay = 1.0f;
	for (int i = 0; i < NUM_SAMPLES; i++)
  	{
    	// Step sample location along ray.
    	texCoord -= deltaTexCoord;
    	// Retrieve sample at new location.
		
		half3 sampled = sceneTexture.Sample(LinearWrapSampler, texCoord).rgb;
		//half3 sampled = half3(0.5f, 1.0f, 0.9f);
    	// Apply sample attenuation scale/decay factors.
    	//sampled *= illuminationDecay * Constants.Weight;
    	sampled *= illuminationDecay * 0.025f;
    	// Accumulate combined color.
    	color += sampled;
    	// Update exponential decay factor.
    	illuminationDecay *= Constants.IlluminationDecay;
  	}

	outputTexture[DispatchThreadID.xy] = float4(color.rgb, 1.0f);

}

#endif // SCATTERING_HLSL

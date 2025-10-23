#ifndef FILM_EFFECTS_HLSL
#define FILM_EFFECTS_HLSL

#include "../../Common/Common.hlsli"
#include "../../Common/Bindless.hlsli"
#include "FilmEffects_RS.hlsli"

#define DISPATCH_BLOCK 16

struct FilmEffectsParameters
{
	uint InputImageIndex;
	uint OutputImageIndex;
	uint bEnableChromaticAberration;
	uint bEnableLensDistortion;
	uint bEnableFilmGrain;
	
	float LensDistortionIntensity;
};

ConstantBuffer<FilmEffectsParameters> Constants : register(b0);

float2 LensDistortion(float2 UV, float2 Texel)
{
	float2 center = float2(0.5f, 0.5f);
	
	float2 distortionVector = UV - center;
	float distortionRadius = length(distortionVector);
	float distortionFactor = 1.0f + Constants.LensDistortionIntensity * distortionRadius * distortionRadius;
	UV = center + distortionVector * distortionFactor;
	
	return UV;
}

SamplerState TexSampler : register(s0);

[RootSignature(FILM_EFFECTS_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4>	sceneTex = GetTexture(Constants.InputImageIndex);
	RWTexture2D<float4> sceneTexture = GetRWTexture<float4>(Constants.OutputImageIndex);
	// https://github.com/mateeeeeee/Adria/blob/master/Adria/Resources/Shaders/Postprocess/FilmEffects.hlsl
	const float2 textureSize = GetTextureSize(sceneTexture);
	const float2 texelSize = GetTexelSize(textureSize);
	
	float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) / textureSize;
	
	float3 scene = sceneTexture[DispatchThreadID.xy].rgb;
	
	float2 uv = float2(0.0f, 0.0f);
	if (Constants.bEnableLensDistortion)
	{
		//texCoord = LensDistortion(scene.xy);
		uv = LensDistortion(uv.xy, texelSize);
	}
	
	float3 color = sceneTex.Sample(TexSampler, uv).rgb;
	sceneTexture[DispatchThreadID.xy] = float4(color, 1.0f);
	//sceneTexture[texCoord] = float4(scene, 1.0f);

}

#endif // FILM_EFFECTS_HLSL

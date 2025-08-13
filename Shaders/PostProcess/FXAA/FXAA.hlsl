#ifndef FXAA_HLSL
#define FXAA_HLSL

#include "../../Common/Common.hlsli"
#include "FXAA_RS.hlsli"

#define DISPATCH_BLOCK 8

struct Parameters
{
	uint	SceneImageIndex;
	uint	DebugImageIndex;
	float	Quality;
	float	EdgeThreshold;
	float	EdgeThresholdMin;
};

ConstantBuffer<Parameters> Constants : register(b0);

SamplerState texSampler : register(s0);

[RootSignature(FXAA_ROOT_SIG)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	Texture2D<float4>	sceneTexture  = GetBindlessTexture(Constants.SceneImageIndex);
	RWTexture2D<float4> outputTexture = GetBindlessRWTexture<float4>(Constants.DebugImageIndex);
	
	const float2 textureSize = GetTextureSize(sceneTexture);
	const float2 uv = ((float2) DispatchThreadID.xy + 0.5f) * 1.0f / textureSize;

	const float3 colorNW = sceneTexture.Sample(texSampler, uv + float2(-1.0f, -1.0f) / textureSize, 0).rgb;
	const float3 colorNE = sceneTexture.Sample(texSampler, uv + float2(+1.0f, -1.0f) / textureSize, 0).rgb;
	const float3 colorSW = sceneTexture.Sample(texSampler, uv + float2(-1.0f, +1.0f) / textureSize, 0).rgb;
	const float3 colorSE = sceneTexture.Sample(texSampler, uv + float2(+1.0f, +1.0f) / textureSize, 0).rgb;
	const float3 colorM  = sceneTexture.Sample(texSampler, uv, 0).rgb;

	const float3 luma = float3(0.299f, 0.587f, 0.114f);
	
	const float lumaNW = dot(colorNW, luma);
	const float lumaNE = dot(colorNE, luma);
	const float lumaSW = dot(colorSW, luma);
	const float lumaSE = dot(colorSE, luma);
	const float lumaM  = dot(colorM, luma);

	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	float2 direction;
	direction.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	direction.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float directionReduce = max(
		(lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * Constants.EdgeThreshold), Constants.EdgeThresholdMin);

	float rcpDirMin = 1.0 / (min(abs(direction.x), abs(direction.y)) + directionReduce);

	direction = min(float2(Constants.Quality, Constants.Quality), max(float2(-Constants.Quality, -Constants.Quality),direction * rcpDirMin)) / textureSize;

	float3 rgbA = (1.0 / 2.0) *
		(
			sceneTexture.Sample(texSampler, uv + direction * (1.0 / 3.0 - 0.5), 0).rgb +
			sceneTexture.Sample(texSampler, uv + direction * (2.0 / 3.0 - 0.5), 0).rgb
			);

	float3 rgbB = rgbA * (1.0 / 2.0) + (1.0 / 4.0) *
		(sceneTexture.Sample(texSampler, uv + direction * (0.0 / 3.0 - 0.5), 0).rgb + sceneTexture.Sample(texSampler, uv + direction * (3.0 / 3.0 - 0.5), 0).rgb);

	float lumaB = dot(rgbB, luma);

	float3 finalColor;
	if ((lumaB < lumaMin) || (lumaB > lumaMax))
	{
		finalColor.xyz = rgbA;
	}
	else
	{
		finalColor.xyz = rgbB;
	}
	
	outputTexture[DispatchThreadID.xy] = float4(finalColor, 1.0f);
}

#endif // FXAA_HLSL

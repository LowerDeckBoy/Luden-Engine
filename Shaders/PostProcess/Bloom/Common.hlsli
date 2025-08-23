#ifndef BLOOM_COMMON_HLSLI
#define BLOOM_COMMON_HLSLI

#include "../../Common/Bindless.hlsli"
#include "../../Common/Common.hlsli"

#define DISPATCH_BLOCK 8

struct BloomParameters
{
	uint	EmissiveImageIndex;
	uint	LightImageIndex;
	uint	SceneImageIndex;
	float	Threshold;
	float	ThresholdKnee;
	float	Intensity;
	float	Gamma;
	uint	MipIndex;
};

ConstantBuffer<BloomParameters> Constants : register(b0);

SamplerState TexSampler : register(s0, space0);

#endif // BLOOM_COMMON_HLSLI

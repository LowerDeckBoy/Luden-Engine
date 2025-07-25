#ifndef BLOOM_COMMON_HLSLI
#define BLOOM_COMMON_HLSLI

#define DISPATCH_GROUP 8

struct BloomConstants
{
	uint	BaseColorIndex;
	uint	LightImageIndex;
	uint	SceneImageIndex;
	float	Threshold;
	float	Intensity;
	float	Exposure;
	float	Gamma;
	uint	MipIndex;
};

ConstantBuffer<BloomConstants> Constants : register(b0);

SamplerState linearClampSampler : register(s0);


#endif // BLOOM_COMMON_HLSLI

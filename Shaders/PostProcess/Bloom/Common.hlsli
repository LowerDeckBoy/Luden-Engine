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

float Luminance(float3 Color)
{
	return dot(Color, float3(0.2126729f, 0.7151522f, 0.0721750f));
}


#endif // BLOOM_COMMON_HLSLI

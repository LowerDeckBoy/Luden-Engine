#ifndef TRANSFORM_HLSLI
#define TRANSFORM_HLSLI

struct Transform
{
	row_major float4x4 WVP;
	row_major float4x4 World;
};

#endif // TRANSFORM_HLSLI

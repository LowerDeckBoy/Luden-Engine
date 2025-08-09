#ifndef TONEMAPPING_HLSLI
#define TONEMAPPING_HLSLI

#include "../../Common/Common.hlsli"
#include "Tonemapping.hlsli"
#include "Tonemapping_RS.hlsli"

#define DISPATCH_BLOCK 8

struct Parameters
{
	uint	SceneImageIndex;
	float	Exposure;
	uint	FilterType;
};

ConstantBuffer<Parameters> Constants : register(b0);

[RootSignature(TONEMAPPING_ROOT_SIG)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> scene = GetBindlessRWTexture<float4>(Constants.SceneImageIndex);
	
	float3 color = scene[DispatchThreadID.xy].rgb;
	scene[DispatchThreadID.xy] = float4(Tonemapping::ApplyTonemapping(color, Constants.Exposure, Constants.FilterType), 1.0f);

}

#endif // TONEMAPPING_HLSLI

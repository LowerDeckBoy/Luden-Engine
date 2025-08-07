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

[RootSignature(FXAA_ROOT_SIG)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	RWTexture2D<float4> debugTexture = GetBindlessRWTexture<float4>(Constants.DebugImageIndex);
	RWTexture2D<float4> sceneTexture = GetBindlessRWTexture<float4>(Constants.SceneImageIndex);
	
	float3 color = sceneTexture[DispatchThreadID.xy].rgb;// * 1.25f;
	debugTexture[DispatchThreadID.xy] = float4(color * 3.25f, 1.0f);

}

#endif // FXAA_HLSL

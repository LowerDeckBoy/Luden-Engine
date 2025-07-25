#ifndef BLOOM_COMBINE_HLSL
#define BLOOM_COMBINE_HLSL

#include "Common.hlsli"

[numthreads(DISPATCH_GROUP, DISPATCH_GROUP, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID )
{
	Texture2D<float4>	bloomImage		= ResourceDescriptorHeap[Constants.MipIndex];
	RWTexture2D<float4> sceneTexture	= ResourceDescriptorHeap[Constants.SceneImageIndex];
	
	float2 textureSize;
	sceneTexture.GetDimensions(textureSize.x, textureSize.y);
	
	const float2 texelSize = 1.0f / textureSize;
	const float2 texCoord = (DispatchThreadID.xy + 0.5f) * texelSize;
	
	float3 scene = sceneTexture[DispatchThreadID.xy].rgb;
	float3 bloom = bloomImage.Sample(linearClampSampler, texCoord).rgb;
	
	sceneTexture[DispatchThreadID.xy] = float4(lerp(scene.rgb, scene.rgb + bloom, Constants.Gamma), 1.0f);
}

#endif // BLOOM_COMBINE_HLSL

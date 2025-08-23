#ifndef BLOOM_COMBINE_HLSL
#define BLOOM_COMBINE_HLSL

#include "Common.hlsli"

[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID )
{
	Texture2D<float4>	bloomImage		= GetTexture(Constants.MipIndex);
	RWTexture2D<float4> sceneTexture	= GetRWTexture<float4>(Constants.SceneImageIndex);
	
	const float2 textureSize = GetTextureSize(sceneTexture);
	
	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}

	const float2 texelSize = GetTexelSize(textureSize);
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;

	float3 scene = sceneTexture[DispatchThreadID.xy].rgb;
	float3 bloom = bloomImage.Sample(TexSampler, texCoord).rgb;
	
	sceneTexture[DispatchThreadID.xy] = float4(lerp(scene.rgb, scene.rgb + bloom, Constants.Gamma), 1.0f);
}

#endif // BLOOM_COMBINE_HLSL

#ifndef BLOOM_COMBINE_HLSL
#define BLOOM_COMBINE_HLSL

#include "Common.hlsli"

[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID )
{
	//Texture2D<float4>	bloomImage		= GetTexture(Constants.MipIndex);
	RWTexture2D<float4>	bloomImage		= GetRWTexture<float4>(Constants.MipIndex);
	RWTexture2D<float4> sceneTexture	= GetRWTexture<float4>(Constants.SceneImageIndex);
	
	const float2 textureSize = GetTextureSize(sceneTexture);
	
	if (DispatchThreadID.x >= textureSize.x || DispatchThreadID.y >= textureSize.y)
	{
		return;
	}

	const float2 texelSize = GetTexelSize(textureSize);
	const float2 texCoord = (float2(DispatchThreadID.xy) + 0.5f) * texelSize;

	//float4 scene = sceneTexture[DispatchThreadID.xy];
	float3 scene = sceneTexture[DispatchThreadID.xy].rgb;
	//float4 bloom = bloomImage.Sample(TexSampler, texCoord) * Constants.Intensity;
	//float4 bloom = bloomImage.Load(uint3(texCoord, 0)) * Constants.Intensity;
	//float3 bloom = bloomImage.Load(uint3(texCoord, 0)).rgb;// * Constants.Intensity;
	float3 bloom = bloomImage[DispatchThreadID.xy].rgb * Constants.Intensity;
	
	sceneTexture[DispatchThreadID.xy].rgb = lerp(scene, scene + bloom, 1.0f);
	//sceneTexture[DispatchThreadID.xy] = lerp(scene, scene + bloom, Constants.Intensity);
}

#endif // BLOOM_COMBINE_HLSL

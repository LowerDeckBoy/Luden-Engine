#ifndef MATERIAL_HLSLI
#define MATERIAL_HLSLI

struct Material
{
	float4	BaseColor;
	float4	EmissiveColor;
	
	float	Metalness;
	float	Roughness;
	float	AlphaCutoff;
	uint	bDoubleSided;
	
	uint	BaseColorIndex;
	uint	NormalIndex;
	uint	MetalRoughnessIndex;
	uint	EmissiveIndex;
};

#endif // MATERIAL_HLSLI

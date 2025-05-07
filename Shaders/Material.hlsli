#ifndef MATERIAL_HLSLI
#define MATERIAL_HLSLI

static const uint ALPHA_MODE_OPAQUE	= 0;
static const uint ALPHA_MODE_MASK   = 1;
static const uint ALPHA_MODE_BLEND	= 2;

struct FMaterial
{
	float4	BaseColorFactor;
	float4	EmissiveFactor;

	float	AlphaCutoff;
	float	Metallic;
	float	Roughness;
	float	IndexOfRefraction;
		
	float	Anisotropy;
	float	Glossiness;
	float	Reflectivity;
	uint	AlphaMode;

	uint	BaseColorIndex;
	uint	NormalIndex;
	uint	MetallicRoughnessIndex;
	uint	EmissiveIndex;

};

#endif // MATERIAL_HLSLI

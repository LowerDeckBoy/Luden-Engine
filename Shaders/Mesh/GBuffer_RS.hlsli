#ifndef GBUFFER_RS_HLSLI
#define GBUFFER_RS_HLSLI

#define GBUFFER_ROOT_SIG "RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |"\
	"DENY_HULL_SHADER_ROOT_ACCESS |"\
	"DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
	"DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
	"CBV(b0, space=0), "\
	"RootConstants(num32BitConstants=7, b1), "\
	"RootConstants(num32BitConstants=20, b2), "\
	"StaticSampler(s0, "\
		"addressU = TEXTURE_ADDRESS_WRAP, "\
		"addressV = TEXTURE_ADDRESS_WRAP, "\
		"filter = FILTER_MAXIMUM_ANISOTROPIC )"

#endif // GBUFFER_RS_HLSLI

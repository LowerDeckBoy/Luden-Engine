#ifndef DEFERRED_RS_HLSLI
#define DEFERRED_RS_HLSLI

#define ROOT_SIG "RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |"\
	"DENY_HULL_SHADER_ROOT_ACCESS |"\
	"DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
	"DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
	"CBV(b0, space=0), "\
	"CBV(b1, space=0), "\
	"RootConstants(num32BitConstants=4, b2), "\
	"StaticSampler(s0, "\
		"addressU = TEXTURE_ADDRESS_WRAP, "\
		"addressV = TEXTURE_ADDRESS_WRAP, "\
		"filter = FILTER_MAXIMUM_ANISOTROPIC )"

#endif // DEFERRED_RS_HLSLI

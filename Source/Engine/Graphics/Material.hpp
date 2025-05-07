#pragma once

#include <Core/Types.hpp>
#include <DirectXMath.h>

namespace Luden
{
	enum EAlphaMode
	{
		Opaque,
		Mask,
		Blend,
	};

	struct Material
	{
		DirectX::XMFLOAT4 BaseColorFactor{};
		DirectX::XMFLOAT4 EmissiveFactor{};

		float AlphaCutoff		= 0.5f;
		float Metallic			= 0.0f;
		float Roughness			= 0.5f;
		float IndexOfRefraction	= 1.0f;
		
		float Anisotropy		= 0.0f;
		float Glossiness		= 0.0f;
		float Reflectivity		= 0.0f;
		EAlphaMode AlphaMode	= Opaque;

		uint32	BaseColorIndex			= 0xFFFFFFFF;
		uint32	NormalIndex				= 0xFFFFFFFF;
		uint32	MetallicRoughnessIndex	= 0xFFFFFFFF;
		uint32	EmissiveIndex			= 0xFFFFFFFF;

	};
} // namespace Luden

#pragma once

#include <Core/Types.hpp>
#include <DirectXMath.h>

namespace Luden
{
	struct Material
	{
		DirectX::XMFLOAT4 BaseColorFactor;
		DirectX::XMFLOAT4 EmissiveFactor;

		f32		AlphaCutoff = 0.5f;
		f32		MetallicFactor = 0.0f;
		f32		RoughnessFactor = 0.5f;
		int32	bDoubleSided = false;

		f32		Clearcoat;
		f32		Glossiness;

		uint32	BaseColorIndex = 0xFFFFFFFF;
		uint32	NormalIndex = 0xFFFFFFFF;
		uint32	MetallicRoughnessIndex = 0xFFFFFFFF;
		uint32	EmissiveIndex = 0xFFFFFFFF;

	};
} // namespace Luden

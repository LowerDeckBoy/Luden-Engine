#pragma once

#include <DirectXMath.h>

namespace Luden::ecs
{
	struct BoundingBoxComponent
	{
		DirectX::XMFLOAT3 Min;
		DirectX::XMFLOAT3 Max;
	};
} // namespace Luden::ecs

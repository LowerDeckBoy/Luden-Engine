#pragma once

#include <DirectXMath.h>

namespace Luden::ecs
{
	struct PointLightComponent
	{
		DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float Intensity = 1.0f;
		// White by default.
		DirectX::XMFLOAT3 Ambient = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		float Range = 30.0f;

		PointLightComponent(DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f))
			: Position(Position)
		{ }
	};
} // namespace Luden::ecs

#pragma once

#include <DirectXMath.h>

namespace Luden::ecs
{
	struct DirectionalLightComponent
	{

	};

	struct PointLightComponent
	{
		DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float Intensity = 1.0f;
		// White by default.
		DirectX::XMFLOAT3 Ambient = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		float Radius = 35.0f;
	};
} // namespace Luden::ecs

#pragma once

#include <DirectXMath.h>

namespace Luden::ecs
{
	struct DirectionalLightComponent
	{
		DirectX::XMFLOAT3 Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
		float Intensity = 1.0f;

		DirectX::XMFLOAT3 Ambient = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		float padding = 0.0f;
	};

	struct PointLightComponent
	{
		// Guizmo test
		DirectX::XMFLOAT4X4 Transform;

		DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float Intensity = 1.0f;
		
		DirectX::XMFLOAT3 Ambient = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		float Radius = 35.0f;
	};

	struct SpotLightComponent
	{
		// Guizmo test
		DirectX::XMFLOAT4X4 Transform;

		DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float Intensity = 1.0f;

		DirectX::XMFLOAT3 Direction = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float InnerCutoff = 12.0f;
		
		DirectX::XMFLOAT3 Ambient = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		float OuterCutoff = 20.0f;
	};

} // namespace Luden::ecs

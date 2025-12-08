#pragma once

#include <DirectXMath.h>

namespace Luden::ecs
{
	struct DirectionalLightComponent
	{
		DirectX::XMFLOAT3 Direction = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float Intensity = 1.0f;

		DirectX::XMFLOAT3 Ambient = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		uint32 bIsVisible = 1;
	};

	struct PointLightComponent
	{
		PointLightComponent(DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f))
			: Position(Position) {}

		// Guizmo test
		DirectX::XMFLOAT4X4 Transform{};

		DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float Intensity = 1.0f;
		
		DirectX::XMFLOAT3 Ambient = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		float Radius = 35.0f;
		uint32 bIsVisible = 1;
	};

	struct SpotLightComponent
	{
		// Guizmo test
		DirectX::XMFLOAT4X4 Transform;

		DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float Intensity = 10.0f;

		DirectX::XMFLOAT3 Direction = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		float InnerCutoff = 20.0f;
		
		DirectX::XMFLOAT3 Ambient = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		float OuterCutoff = 65.0f;
		uint32 bIsVisible = 1;
	};

} // namespace Luden::ecs

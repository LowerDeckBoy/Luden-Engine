#pragma once

#include <Core/Math/Math.hpp>
#include <DirectXMath.h>

namespace Luden
{
	constexpr DirectX::XMVECTOR ForwardVector	= DirectX::XMVECTOR(0.0f, 0.0f, 1.0f, 0.0f);
	constexpr DirectX::XMVECTOR RightVector		= DirectX::XMVECTOR(1.0f, 0.0f, 0.0f, 0.0f);
	constexpr DirectX::XMVECTOR UpVector		= DirectX::XMVECTOR(0.0f, 1.0f, 0.0f, 0.0f);
}

namespace Luden::ecs
{
	struct CameraComponent
	{
		CameraComponent()
			: Position(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f)),
			LookAt(DirectX::XMMATRIX())
		{ }

		DirectX::XMFLOAT3 Position;
		DirectX::XMMATRIX LookAt;

		f32 FoV			= Math::ConvertToRadians(45.0f);
		f32 NearZ		= 0.1f;
		f32 FarZ		= 10000.0f;
		f32 Speed		= 25.0f;
		f32 Sensivity	= 0.1f;

		void Reset()
		{
			Position	= DirectX::XMFLOAT3(0.0f, 1.0f, -10.0f);
			FoV			= Math::ConvertToRadians(45.0f);
			Speed		= 25.0f;
			Sensivity	= 0.1f;
		}
	};
} // namespace Luden::ecs

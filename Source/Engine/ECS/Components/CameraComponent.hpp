#pragma once

#include <DirectXMath.h>

constexpr DirectX::XMVECTOR ForwardVector	= DirectX::XMVECTOR(0.0f, 0.0f, 1.0f, 0.0f);
constexpr DirectX::XMVECTOR RightVector		= DirectX::XMVECTOR(1.0f, 0.0f, 0.0f, 0.0f);
constexpr DirectX::XMVECTOR UpVector		= DirectX::XMVECTOR(0.0f, 1.0f, 0.0f, 0.0f);

namespace Luden::ecs
{
	struct CameraComponent
	{
		DirectX::XMFLOAT3 Position	= DirectX::XMFLOAT3(0.0f, 1.0f, -10.0f);
		DirectX::XMMATRIX LookAt	= DirectX::XMMATRIX();

		float FoV		= 45.0f;
		float zNear		= 0.1f;
		float zFar		= 25000.0f;
		float Speed		= 25.0f;
		float Sensivity = 0.1f;

		void Reset()
		{
			Position	= DirectX::XMFLOAT3(0.0f, 1.0f, -10.0f);
			FoV			= 45.0f;
			Speed		= 25.0f;
			Sensivity	= 0.1f;
		}
	};
} // namespace Luden::ecs

#pragma once

#include <Core/Math/Math.hpp>
#include <DirectXMath.h>

namespace Luden::ecs
{
	// Use to convert object's rotation from degrees to radians.
	// Mostly for editor usage.
	const DirectX::XMVECTOR RadiansVector = DirectX::XMVectorSet(Math::Deg2Rad, Math::Deg2Rad, Math::Deg2Rad, 1.0f);

	struct TransformComponent
	{
		TransformComponent() 
			: Translation(0.0f, 0.0f, 0.0f), Rotation(0.0f, 0.0f, 0.0f), Scale(1.0f, 1.0f, 1.0f) 
		{
		}
		TransformComponent(DirectX::XMFLOAT3 Position)
			: Translation(Position), Rotation(0.0f, 0.0f, 0.0f), Scale(1.0f, 1.0f, 1.0f)
		{
		}
		TransformComponent(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Rotation)
			: Translation(Position), Rotation(Rotation), Scale(1.0f, 1.0f, 1.0f)
		{
		}
		TransformComponent(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Rotation, DirectX::XMFLOAT3 Scale)
			: Translation(Position), Rotation(Rotation), Scale(Scale)
		{
		}

		DirectX::XMFLOAT3 Translation;
		DirectX::XMFLOAT3 Rotation;
		DirectX::XMFLOAT3 Scale;

		DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();

		void Update()
		{
			WorldMatrix =
				DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&Scale)) *
				DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMVectorMultiply((XMLoadFloat3(&Rotation)), RadiansVector)) *
				DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&Translation));
		}

		void Reset()
		{
			Translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			Rotation	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			Scale		= DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

			Update();
		}
	};
} // namespace Luden::ecs

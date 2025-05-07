#pragma once

#include <Core/Math/Math.hpp>
#include <DirectXMath.h>

namespace Luden::ecs
{
	// Use to convert object's rotation from degrees to radians.
	// Mostly for editor usage.
	const DirectX::XMVECTOR RadiansVector = DirectX::XMVectorSet(Math::Deg2Rad, Math::Deg2Rad, Math::Deg2Rad, 1.0);

	struct TransformComponent
	{
		TransformComponent() 
			: Translation(0.0f, 0.0f, 0.0f), Rotation(0.0f, 0.0f, 0.0f, 1.0f), Scale(1.0f, 1.0f, 1.0f) 
		{
		}
		TransformComponent(DirectX::XMFLOAT3 Position)
			: Translation(Position), Rotation(0.0f, 0.0f, 0.0f, 1.0f), Scale(1.0f, 1.0f, 1.0f)
		{
		}
		TransformComponent(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT4 Rotation)
			: Translation(Position), Rotation(Rotation), Scale(1.0f, 1.0f, 1.0f)
		{
		}
		TransformComponent(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT4 Rotation, DirectX::XMFLOAT3 Scale)
			: Translation(Position), Rotation(Rotation), Scale(Scale)
		{
		}

		DirectX::XMFLOAT3 Translation;
		DirectX::XMFLOAT4 Rotation;
		DirectX::XMFLOAT3 Scale;

		DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();

		// Flag as *true* if any value changes, then update matrices.
		bool bDirty = false;

		// Update World (aka Model) transformation as S * R * T
		void Update()
		{
			WorldMatrix =
				DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Scale)) *
				DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMVectorMultiply((XMLoadFloat4(&Rotation)), RadiansVector)) *
				DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Translation));

			bDirty = false;
			
		}

		void Decompose(DirectX::XMMATRIX Matrix)
		{
			auto scale			= DirectX::XMLoadFloat3(&Scale);
			auto rotation		= DirectX::XMLoadFloat4(&Rotation);
			auto translation	= DirectX::XMLoadFloat3(&Translation);

			DirectX::XMMatrixDecompose(
				&scale,
				&rotation,
				&translation,
				Matrix);

			DirectX::XMStoreFloat3(&Scale, scale);
			DirectX::XMStoreFloat4(&Rotation, rotation);
			DirectX::XMStoreFloat3(&Translation, translation);
		}

		// Returns S * R * T matrix
		DirectX::XMMATRIX GetLocalTransform() const
		{
			return	DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Scale)) *
					DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMVectorMultiply((XMLoadFloat4(&Rotation)), RadiansVector)) *
					DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Translation));
		}

		void Reset()
		{
			Translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			Rotation	= DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			Scale		= DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

			Update();
		}
	};
} // namespace Luden::ecs

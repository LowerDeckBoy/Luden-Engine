#pragma once

#define _XM_SSE4_INTRINSICS_
#include <DirectXMath.h>

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <Core/Types.hpp>
#include "ECS/Components/BoundingBoxComponent.hpp"

namespace Luden::Platform
{
	class Window;
}

namespace Luden
{
	class SceneCamera
	{
	public:
		SceneCamera(Platform::Window* pWindow);
		~SceneCamera();

		// Update Projection matrix.
		void Resize();
		void Tick(f64 DeltaTime);
		void Update();
		
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Target;
		DirectX::XMFLOAT3 Up;
		
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;

		DirectX::XMFLOAT4X4 InversedView;
		DirectX::XMFLOAT4X4 InversedProjection;

		DirectX::XMMATRIX GetViewProjection() const;

		f32 zNear		= 1.0f;
		f32 zFar		= 5000.0f;

		f32 AspectRatio = 1.0f;
		f32 FieldOfView = 45.0f;

		f32 Pitch		= 0.0f;
		f32 Yaw			= 0.0f;
		
		f32 CameraSpeed = 15.0f;

		// Check whether BoundingBox is inside of Camera's frustrum.
		bool IsInFrustrum(ecs::BoundingBoxComponent& BoundingBox, DirectX::XMFLOAT4X4 Transform);

		struct Plane
		{
			DirectX::XMFLOAT3 Normal;
			float Distance;
		};
		//std::array<Plane, 6> Frustrums;

	private:
		Platform::Window* m_ParentWindow;

		DirectX::XMFLOAT4X4 m_RotationMatrix		= DirectX::XMFLOAT4X4();

		DirectX::XMFLOAT3 m_Forward					= DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		DirectX::XMFLOAT3 m_Right					= DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 m_Upward					= DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

		DirectX::XMFLOAT3 const m_DefaultPosition	= DirectX::XMFLOAT3(0.0f, 1.0f, -20.0f);
		//DirectX::XMFLOAT3 const m_DefaultTarget		= DirectX::XMFLOAT3(0.0f, 20.0f, 0.0f);
		DirectX::XMFLOAT3 const m_DefaultTarget		= DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		DirectX::XMFLOAT3 const m_DefaultUp			= DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

		DirectX::XMFLOAT3 const m_DefaultForward	= DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		DirectX::XMFLOAT3 const m_DefaultRight		= DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 const m_DefaultUpward		= DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

		float MoveForwardBack	= 0.0f;
		float MoveRightLeft		= 0.0f;
		float MoveUpDown		= 0.0f;

	private:
		// Temporarly using DInput for simplicity sake.
		inline static IDirectInputDevice8*	DxKeyboard{};
		inline static IDirectInputDevice8*	DxMouse{};
		inline static LPDIRECTINPUT8		DxInput{};
		inline static DIMOUSESTATE			DxLastMouseState{};

	};
} // namespace Luden

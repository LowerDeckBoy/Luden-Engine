#pragma once

#define _XM_SSE4_INTRINSICS_
#include <DirectXMath.h>
#include <DirectXCollision.h>

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <array>

#include <Core/Types.hpp>
#include "ECS/Components/BoundingBoxComponent.hpp"

namespace Luden::Platform
{
	class Window;
}

namespace Luden
{
	struct FMeshletBounds;

	enum EFrustumSide : usize
	{
		Left, 
		Right,
		Bottom,
		Top,
		Back, 
		Front
	};

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
		DirectX::XMFLOAT4 Target;
		DirectX::XMFLOAT3 Up;
		
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;

		DirectX::XMFLOAT4X4 InversedView;
		DirectX::XMFLOAT4X4 InversedProjection;

		//DirectX::BoundingFrustum Frustum;

		bool IsInsideFrustum(ecs::BoundingBoxComponent& AABB);
		std::array<DirectX::XMFLOAT4, 6> FrustumPlanes;

		void ConstructFrustum(const DirectX::XMMATRIX& Transformation);

		DirectX::XMMATRIX GetView();
		DirectX::XMMATRIX GetProjection();
		DirectX::XMMATRIX GetViewProjection() const;

		f32 zNear		= 0.01f;
		f32 zFar		= 10000.0f;

		f32 AspectRatio = 1.0f;
		f32 FieldOfView = 45.0f;

		f32 Pitch		= 0.0f;
		f32 Yaw			= 0.0f;
		
		f32 CameraSpeed = 15.0f;

		// TODO:
		// Check whether BoundingBox is inside of Camera's frustrum.
		bool IsInFrustrum(ecs::BoundingBoxComponent& BoundingBox, DirectX::XMFLOAT4X4 Transform);

		bool IsInFrustrum(FMeshletBounds& BoundingBox, DirectX::XMFLOAT4X4 Transform);

	private:
		Platform::Window* m_ParentWindow;

		DirectX::XMFLOAT4X4 m_RotationMatrix		= DirectX::XMFLOAT4X4();

		DirectX::XMFLOAT3 m_Forward					= DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		DirectX::XMFLOAT3 m_Right					= DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 m_Upward					= DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

		DirectX::XMFLOAT3 const m_DefaultPosition	= DirectX::XMFLOAT3(0.0f, 1.0f, -15.0f);
		DirectX::XMFLOAT4 const m_DefaultTarget		= DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
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

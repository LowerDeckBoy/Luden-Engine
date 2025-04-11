#include "SceneCamera.hpp"
#include <Platform/Window.hpp>
#include <array>
#include <Core/Logger.hpp>
#include <Core/Math/Math.hpp>
#pragma comment(lib, "dinput8")

//#include "Graphics/Mesh.hpp"

using namespace DirectX;

namespace Luden
{
	SceneCamera::SceneCamera(Platform::Window* pWindow)
		: m_ParentWindow(pWindow)
	{
		Position	= m_DefaultPosition;
		Target		= m_DefaultTarget;
		Up			= m_DefaultUp;

		XMStoreFloat4x4(&View, XMMatrixLookAtLH(XMLoadFloat3(&Position), XMLoadFloat4(&Target), XMLoadFloat3(&Up)));
		AspectRatio = (f32)pWindow->Width / pWindow->Height;
		XMStoreFloat4x4(&Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(FieldOfView), AspectRatio, zNear, zFar));

		DirectInput8Create(pWindow->Instance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&DxInput), NULL);
		DxInput->CreateDevice(GUID_SysKeyboard, &DxKeyboard, NULL);
		DxKeyboard->SetDataFormat(&c_dfDIKeyboard);
		DxKeyboard->SetCooperativeLevel(pWindow->Handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		DxInput->CreateDevice(GUID_SysMouse, &DxMouse, NULL);
		DxMouse->SetDataFormat(&c_dfDIMouse);
		DxMouse->SetCooperativeLevel(pWindow->Handle, DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

		//Frustum.Origin = Target;
	}

	SceneCamera::~SceneCamera()
	{
		
	}

	void SceneCamera::Resize()
	{
		AspectRatio = (f32)m_ParentWindow->Width / (f32)m_ParentWindow->Height;
		XMStoreFloat4x4(&Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(FieldOfView), AspectRatio, zNear, zFar));

		ConstructFrustum(GetViewProjection());

		Update();
	}

	void SceneCamera::Tick(f64 DeltaTime)
	{
		DIMOUSESTATE mouseState{};
		constexpr int keys = 256;
		std::array<BYTE, keys> keyboardState{};

		DxKeyboard->Acquire();
		DxMouse->Acquire();

		DxMouse->GetDeviceState(sizeof(mouseState), reinterpret_cast<LPVOID>(&mouseState));

		// If RMB is not held - skip mouse and keyboard camera controls
		if (!mouseState.rgbButtons[1])
		{
			m_ParentWindow->OnCursorShow();

			return;
		}

		// Adjust camera speed on mouse scroll.
		if (mouseState.lZ)
		{
			CameraSpeed = Math::Clamp(CameraSpeed - (-mouseState.lZ * 0.01f), 1.0f, 250.0f);
		}

		DxKeyboard->GetDeviceState(sizeof(keyboardState), reinterpret_cast<LPVOID>(&keyboardState));

		constexpr int state = 0x80;

		m_ParentWindow->OnCursorHide();

		if ((mouseState.lX != DxLastMouseState.lX) || (mouseState.lY != DxLastMouseState.lY))
		{
			Yaw		+= mouseState.lX * 0.001f;
			Pitch	+= mouseState.lY * 0.001f;
			DxLastMouseState = mouseState;
		}

		const f32 intensity = static_cast<f32>(DeltaTime * CameraSpeed * 0.001f);

		if (keyboardState.at(DIK_W) & state)
		{
			MoveForwardBack += intensity;
		}

		if (keyboardState.at(DIK_S) & state)
		{
			MoveForwardBack -= intensity;
		}

		if (keyboardState.at(DIK_A) & state)
		{
			MoveRightLeft -= intensity;
		}

		if (keyboardState.at(DIK_D) & state)
		{
			MoveRightLeft += intensity;
		}

		if (keyboardState.at(DIK_Q) & state)
		{
			MoveUpDown -= intensity;
		}

		if (keyboardState.at(DIK_E) & state)
		{
			MoveUpDown += intensity;
		}

		Update();
		ConstructFrustum(GetViewProjection());
	}

	void SceneCamera::Update()
	{
		XMVECTOR position	= XMLoadFloat3(&Position);
		XMVECTOR forward	= XMLoadFloat3(&m_Forward);
		XMVECTOR right		= XMLoadFloat3(&m_Right);
		XMVECTOR up			= XMLoadFloat3(&Up);
		XMVECTOR target		= XMLoadFloat4(&Target);

		XMMATRIX rotationMatrix = XMLoadFloat4x4(&m_RotationMatrix);
		rotationMatrix = XMMatrixRotationRollPitchYaw(Pitch, Yaw, 0.0f);
		target = XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&m_DefaultForward), rotationMatrix));

		const XMMATRIX rotation = XMMatrixRotationY(Yaw);

		forward = XMVector3TransformCoord(XMLoadFloat3(&m_DefaultForward), rotation);
		right = XMVector3TransformCoord(XMLoadFloat3(&m_DefaultRight), rotation);
		up = XMVector3TransformCoord(XMLoadFloat3(&m_DefaultUp), rotation);

		position += (MoveForwardBack * forward);
		position += (MoveRightLeft * right);
		position += (MoveUpDown * up);

		MoveForwardBack = 0.0f;
		MoveRightLeft	= 0.0f;
		MoveUpDown		= 0.0f;

		target += position;

		XMStoreFloat4x4(&View, XMMatrixLookAtLH(position, target, up));

		// Store vector and matrices
		XMStoreFloat3(&m_Forward, forward);
		XMStoreFloat3(&m_Right, right);
		XMStoreFloat3(&Up, up);
		XMStoreFloat4x4(&m_RotationMatrix, rotationMatrix);
		XMStoreFloat4(&Target, target);
		XMStoreFloat3(&Position, position);

	}

	bool SceneCamera::IsInsideFrustum(ecs::BoundingBoxComponent& AABB)
	{
		// Corners
		std::array<DirectX::XMFLOAT4, 8> corners = {
			XMFLOAT4(AABB.Min.x, AABB.Min.y, AABB.Min.z, 1.0f),
			XMFLOAT4(AABB.Max.x, AABB.Min.y, AABB.Min.z, 1.0f),
			XMFLOAT4(AABB.Max.x, AABB.Max.y, AABB.Min.z, 1.0f),
			XMFLOAT4(AABB.Min.x, AABB.Max.y, AABB.Min.z, 1.0f),

			XMFLOAT4(AABB.Min.x, AABB.Min.y, AABB.Max.z, 1.0f),
			XMFLOAT4(AABB.Max.x, AABB.Min.y, AABB.Max.z, 1.0f),
			XMFLOAT4(AABB.Max.x, AABB.Max.y, AABB.Max.z, 1.0f),
			XMFLOAT4(AABB.Min.x, AABB.Max.y, AABB.Max.z, 1.0f)
		};

		constexpr float EPSILON = 0.000002f;
		const XMVECTOR V_EPSILON = XMVectorSet(EPSILON, EPSILON, EPSILON, EPSILON);

		for (int p = 0; p < 6; ++p)	// for each plane
		{
			bool bInside = false;
			for (const XMFLOAT4& f4Point : corners)
			{
				XMVECTOR vPoint = XMLoadFloat4(&f4Point);
				XMVECTOR vPlane = XMLoadFloat4(&FrustumPlanes.at(p));
				XMVECTOR cmp = XMVectorGreater(XMVector4Dot(vPoint, vPlane), V_EPSILON);
				if (bInside = cmp.m128_u32[0]) // is point inside frustum ?
				{
					break;
				}
			}
			if (!bInside) // if all the BB points are outside the frustum plane
				return false;
		}

		return true;
	}

	// https://github.com/vilbeyli/VQEngine/blob/master/Source/Engine/CullingData.h
	void SceneCamera::ConstructFrustum(const DirectX::XMMATRIX& Transformation)
	{
		const auto& m = Transformation;

		FrustumPlanes.at(EFrustumSide::Left) = DirectX::XMFLOAT4(
			m.r[0].m128_f32[3] + m.r[0].m128_f32[0],
			m.r[1].m128_f32[3] + m.r[1].m128_f32[0],
			m.r[2].m128_f32[3] + m.r[2].m128_f32[0],
			m.r[3].m128_f32[3] + m.r[3].m128_f32[0]
		);
		
		FrustumPlanes.at(EFrustumSide::Right) = DirectX::XMFLOAT4(
			m.r[0].m128_f32[3] - m.r[0].m128_f32[0],
			m.r[1].m128_f32[3] - m.r[1].m128_f32[0],
			m.r[2].m128_f32[3] - m.r[2].m128_f32[0],
			m.r[3].m128_f32[3] - m.r[3].m128_f32[0]
		);

		FrustumPlanes.at(EFrustumSide::Top) = DirectX::XMFLOAT4(
			m.r[0].m128_f32[3] - m.r[0].m128_f32[1],
			m.r[1].m128_f32[3] - m.r[1].m128_f32[1],
			m.r[2].m128_f32[3] - m.r[2].m128_f32[1],
			m.r[3].m128_f32[3] - m.r[3].m128_f32[1]
		);

		FrustumPlanes.at(EFrustumSide::Bottom) = DirectX::XMFLOAT4(
			m.r[0].m128_f32[3] + m.r[0].m128_f32[1],
			m.r[1].m128_f32[3] + m.r[1].m128_f32[1],
			m.r[2].m128_f32[3] + m.r[2].m128_f32[1],
			m.r[3].m128_f32[3] + m.r[3].m128_f32[1]
		);

		FrustumPlanes.at(EFrustumSide::Back) = DirectX::XMFLOAT4(
			m.r[0].m128_f32[2],
			m.r[1].m128_f32[2],
			m.r[2].m128_f32[2],
			m.r[3].m128_f32[2]
		);

		FrustumPlanes.at(EFrustumSide::Front) = DirectX::XMFLOAT4(
			m.r[0].m128_f32[3] - m.r[0].m128_f32[2],
			m.r[1].m128_f32[3] - m.r[1].m128_f32[2],
			m.r[2].m128_f32[3] - m.r[2].m128_f32[2],
			m.r[3].m128_f32[3] - m.r[3].m128_f32[2]
		);

		// Normalize
		for (int32 side = 0; side < 6; ++side)
		{
			DirectX::XMVECTOR vPlane = DirectX::XMLoadFloat4(&FrustumPlanes.at(side));
			DirectX::XMVECTOR vLen = DirectX::XMVectorSqrt(DirectX::XMVector3Dot(vPlane, vPlane));
			vPlane = DirectX::XMVectorMultiply(vPlane, DirectX::XMVectorReciprocal(vLen));
			DirectX::XMStoreFloat4(&FrustumPlanes.at(side), vPlane);
		}


	}

	DirectX::XMMATRIX SceneCamera::GetView()
	{
		return DirectX::XMLoadFloat4x4(&View);
	}

	DirectX::XMMATRIX SceneCamera::GetProjection()
	{
		return DirectX::XMLoadFloat4x4(&Projection);
	}

	DirectX::XMMATRIX SceneCamera::GetViewProjection() const
	{
		return DirectX::XMLoadFloat4x4(&View) * DirectX::XMLoadFloat4x4(&Projection);
	}

	/*
	bool SceneCamera::IsInFrustrum(ecs::BoundingBoxComponent& BoundingBox, DirectX::XMFLOAT4X4 Transform)
	{
		XMFLOAT4 aabbCorners[8] = {
		{ BoundingBox.Min.x, BoundingBox.Min.y, BoundingBox.Min.z, 1.0 }, // x y z
		{ BoundingBox.Max.x, BoundingBox.Min.y, BoundingBox.Min.z, 1.0 }, // X y z
		{ BoundingBox.Min.x, BoundingBox.Max.y, BoundingBox.Min.z, 1.0 }, // x Y z
		{ BoundingBox.Max.x, BoundingBox.Max.y, BoundingBox.Min.z, 1.0 }, // X Y z

		{ BoundingBox.Min.x, BoundingBox.Min.y, BoundingBox.Max.z, 1.0 }, // x y Z
		{ BoundingBox.Max.x, BoundingBox.Min.y, BoundingBox.Max.z, 1.0 }, // X y Z
		{ BoundingBox.Min.x, BoundingBox.Max.y, BoundingBox.Max.z, 1.0 }, // x Y Z
		{ BoundingBox.Max.x, BoundingBox.Max.y, BoundingBox.Max.z, 1.0 }, // X Y Z
		};

		//Frustum.Origin = Target;
		//Frustum.
		Frustum.Near = 0.1f;
		Frustum.CreateFromMatrix(Frustum, XMLoadFloat4x4(&Projection));
		std::array<DirectX::XMFLOAT3, 8> corners{};
		Frustum.GetCorners(corners.data());

		auto within = [&](float negW, float target, float posW)
		{
			if (target >= negW && target <= posW)
			{
				return true;
			}
			else
			{
				return false;
			}
		};

		bool inside = false;

		for (size_t corner_idx = 0; corner_idx < 8; corner_idx++)
		{
			// Transform vertex
			XMFLOAT4 corner;
			XMStoreFloat4(&corner, XMVector4Transform(XMLoadFloat4(&aabbCorners[corner_idx]), XMLoadFloat4x4(&Transform)));
			//XMFLOAT4 test = corner * XMMulti XMFLOAT4(corners[corner_idx].x, corners[corner_idx].y, corners[corner_idx].z, 1.0f);
			// Check vertex against clip space bounds
			inside = inside ||
				within(-corner.w, corner.x, corner.w) &&
				within(-corner.w, corner.y, corner.w) &&
				within(0.0f, corner.z, corner.w);
		}
		
		return inside;
	}
	*/

	bool SceneCamera::IsInFrustrum(FMeshletBounds& /* BoundingBox */, DirectX::XMFLOAT4X4 /* Transform */)
	{
		//BoundingBox.ConeAxis[1];
		
		XMMATRIX vp = XMMatrixTranspose(GetViewProjection());
		XMVECTOR planes[6] =
		{
			XMPlaneNormalize(vp.r[3] + vp.r[0]), // Left
			XMPlaneNormalize(vp.r[3] - vp.r[0]), // Right
			XMPlaneNormalize(vp.r[3] + vp.r[1]), // Bottom
			XMPlaneNormalize(vp.r[3] - vp.r[1]), // Top
			XMPlaneNormalize(vp.r[2]          ), // Near
			XMPlaneNormalize(vp.r[3] - vp.r[2]), // Far
		};

		return false;
	}
} // namespace Luden

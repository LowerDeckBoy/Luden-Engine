#include "SceneCamera.hpp"
#include <Platform/Window.hpp>
#include <array>
#include <Core/Logger.hpp>
#include <Core/Math/Math.hpp>
#pragma comment(lib, "dinput8")

using namespace DirectX;

namespace Luden
{
	SceneCamera::SceneCamera(Platform::Window* pWindow)
		: m_ParentWindow(pWindow)
	{
		Position	= m_DefaultPosition;
		Target		= m_DefaultTarget;
		Up			= m_DefaultUp;

		XMStoreFloat4x4(&View, XMMatrixLookAtLH(XMLoadFloat3(&Position), XMLoadFloat3(&Target), XMLoadFloat3(&Up)));
		AspectRatio = (f32)pWindow->Width / pWindow->Height;
		XMStoreFloat4x4(&Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(FieldOfView), AspectRatio, zNear, zFar));

		DirectInput8Create(pWindow->Instance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&DxInput), NULL);
		DxInput->CreateDevice(GUID_SysKeyboard, &DxKeyboard, NULL);
		DxKeyboard->SetDataFormat(&c_dfDIKeyboard);
		DxKeyboard->SetCooperativeLevel(pWindow->Handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		DxInput->CreateDevice(GUID_SysMouse, &DxMouse, NULL);
		DxMouse->SetDataFormat(&c_dfDIMouse);
		DxMouse->SetCooperativeLevel(pWindow->Handle, DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

		Frustum.Origin = Target;
	}

	SceneCamera::~SceneCamera()
	{
		
	}

	void SceneCamera::Resize()
	{
		AspectRatio = (f32)m_ParentWindow->Width / m_ParentWindow->Height;
		XMStoreFloat4x4(&Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(FieldOfView), AspectRatio, zNear, zFar));
		
		Frustum.CreateFromMatrix(Frustum, XMLoadFloat4x4(&Projection));

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
			CameraSpeed = Math::Clamp(CameraSpeed - (-mouseState.lZ * 0.01f), 1.0f, 100.0f);
		}

		DxKeyboard->GetDeviceState(sizeof(keyboardState), reinterpret_cast<LPVOID>(&keyboardState));

		constexpr int state = 0x80;

		m_ParentWindow->OnCursorHide();

		if ((mouseState.lX != DxLastMouseState.lX) || (mouseState.lY != DxLastMouseState.lY))
		{
			Yaw += mouseState.lX * 0.001f;
			Pitch += mouseState.lY * 0.001f;
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
	}

	void SceneCamera::Update()
	{
		XMVECTOR position	= XMLoadFloat3(&Position);
		XMVECTOR forward	= XMLoadFloat3(&m_Forward);
		XMVECTOR right		= XMLoadFloat3(&m_Right);
		XMVECTOR up			= XMLoadFloat3(&Up);
		XMVECTOR target		= XMLoadFloat3(&Target);

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
		XMStoreFloat3(&Target, target);
		XMStoreFloat3(&Position, position);

	}

	DirectX::XMMATRIX SceneCamera::GetViewProjection() const
	{
		return DirectX::XMLoadFloat4x4(&View) * DirectX::XMLoadFloat4x4(&Projection);
	}

	bool SceneCamera::IsInFrustrum(ecs::BoundingBoxComponent& BoundingBox, DirectX::XMFLOAT4X4 Transform)
	{

		std::array<DirectX::XMFLOAT3, 8> corners{};
		Frustum.GetCorners(corners.data());


		return false;
	}
} // namespace Luden

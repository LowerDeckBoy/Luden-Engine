#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class D3D12RHI;
	class ShaderCompiler;
	class SceneCamera;

	class Skydome
	{
	public:
		Skydome(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, Scene* pScene);
		~Skydome();

		void Render(Frame& CurrentFrame, SceneCamera* pCamera, DirectX::XMFLOAT3 SunPosition);

		struct
		{
			DirectX::XMMATRIX World;
			DirectX::XMMATRIX View;
			DirectX::XMMATRIX Projection;
		} SkyConstants{};

		struct
		{
			DirectX::XMFLOAT4 SkyColor = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
			DirectX::XMFLOAT4 SunColor = DirectX::XMFLOAT4(1.0f, 0.757f, 0.616f, 1.0f);
			DirectX::XMFLOAT3 CameraPosition;
			float padding;
			DirectX::XMFLOAT3 SunPosition;
			float padding2;
		} SkyParameters{};

		double RenderTime = 0.0;

		D3D12RenderTexture DebugRenderTarget;
	private:
		D3D12RHI* m_D3D12RHI;
		D3D12Pipeline m_PSO;

		uint32 m_IndexBuffer;

		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		ecs::TransformComponent m_Transform{};

	};
}

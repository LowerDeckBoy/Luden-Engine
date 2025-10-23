#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class D3D12RHI;
	class ShaderCompiler;
	class SceneCamera;

	class Skybox
	{
	public:
		Skybox(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler);
		~Skybox();

		void Render(Frame& CurrentFrame, SceneCamera* pCamera, DirectX::XMFLOAT3 SunPosition);
		void RenderSkydome(Frame& CurrentFrame, SceneCamera* pCamera, DirectX::XMFLOAT3 SunPosition);

		struct
		{
			DirectX::XMMATRIX World;
			DirectX::XMMATRIX View;
			DirectX::XMMATRIX Projection;
		} SkyConstants{};

		struct
		{
			DirectX::XMFLOAT4 SkyColor = DirectX::XMFLOAT4(0.0f, 0.2f, 1.0f, 1.0f);
			DirectX::XMFLOAT4 SunColor = DirectX::XMFLOAT4(1.0f, 0.757f, 0.616f, 1.0f);
			DirectX::XMFLOAT3 CameraPosition;
			float padding;
			DirectX::XMFLOAT3 SunPosition;
			uint32 VertexBufferIndx = 0;
		} SkyParameters{};

		double RenderTime = 0.0;

		D3D12RenderTexture DebugRenderTarget;
	private:
		D3D12RHI* m_D3D12RHI;
		D3D12Pipeline m_PSO;
		
		uint32 m_IndexBuffer;

		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		ecs::TransformComponent m_Transform{};

		struct SphereVertex
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT2 TexCoord;
			DirectX::XMFLOAT3 Normal;
		};

		D3D12Pipeline m_SkydomePSO;
		std::vector<SphereVertex> m_SkydomeVertices;
		std::vector<uint32> m_SkydomeIndices;

		D3D12Buffer SkydomeVertexBuffer;
		D3D12Buffer SkydomeIndexBuffer;

		void BuildSkybox();
		void BuildSkydome();

	};
}

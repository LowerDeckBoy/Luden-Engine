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
			DirectX::XMFLOAT3 SkyColor = DirectX::XMFLOAT3(0.0f, 0.2f, 1.0f);
			float SunSize = 0.03f;
			DirectX::XMFLOAT3 SunColor = DirectX::XMFLOAT3(1.0f, 0.757f, 0.616f);
			float SunBloom = 2.f;
			DirectX::XMFLOAT3 SunPosition;
			uint32 VertexBufferIndex = 0;
			DirectX::XMFLOAT3 CameraPosition;
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

	class ProceduralSky
	{
	public:
		ProceduralSky(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler);
		~ProceduralSky();
		 
		void Initialize(uint32 VerticalCount = 32, uint32 HorizontalCount = 32);

		void Render(Frame& CurrentFrame, SceneCamera* pCamera, DirectX::XMFLOAT3 SunPosition, D3D12RenderTexture* pRenderTarget = nullptr);

		void Resize(uint32 Width, uint32 Height);

		struct
		{
			DirectX::XMMATRIX InversedViewProjection;
		} SkyConstants{};

		struct
		{
			DirectX::XMFLOAT3 SkyColor = DirectX::XMFLOAT3(0.0f, 0.2f, 1.0f);
			float Rayleigh = 2.0f;
			DirectX::XMFLOAT3 SunColor = DirectX::XMFLOAT3(1.0f, 0.757f, 0.616f);
			//DirectX::XMFLOAT3 HorizonColor = DirectX::XMFLOAT3(1.0f, 0.757f, 0.616f);
			float MieCoefficient = 0.03f;
			DirectX::XMFLOAT3 SunPosition = DirectX::XMFLOAT3(0.0f, 1.0f, 6.28f);
			float Turbidity = 0.8f;
			DirectX::XMFLOAT3 CameraPosition;
			float Luminance = 1.2f;
			float MieDirectionalG = 0.90f;
			//float SunIntensity = 1.0f;
			uint32 VertexBufferIndex;
		} SkyParameters{};

		D3D12RenderTexture DebugRenderTarget;

		double RenderTime = 0;

	private:
		D3D12RHI* m_D3D12RHI = nullptr;
		D3D12Pipeline m_PSO;

		struct SkyVertex
		{
			DirectX::XMFLOAT2 Position;
		};

		std::vector<SkyVertex> m_Vertices;
		std::vector<uint32> m_Indices;

		D3D12Buffer m_VertexBuffer;
		D3D12Buffer m_IndexBuffer;

	};

	// https://sebh.github.io/publications/egsr2020.pdf
	class SkyTest
	{
	public:
		SkyTest(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~SkyTest();
	
		void Render(Frame& CurrentFrame, SceneCamera* pCamera, DirectX::XMFLOAT3 SunPosition);
		void Resize(uint32 Width, uint32 Height);

		void PrecomputeTransmittance(Frame& CurrentFrame);
		void PrecomputeMultiscatter(Frame& CurrentFrame);

		D3D12RenderTexture* TransmittanceTexture;

		struct
		{
			uint32 TransmittanceIndex;
		} PushConstants{};

	private:
		D3D12RHI* m_D3D12RHI = nullptr;

		D3D12Pipeline m_TransmittancePSO;


	};

} // namespace Luden

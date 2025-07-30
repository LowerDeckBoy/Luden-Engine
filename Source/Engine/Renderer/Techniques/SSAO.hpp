#pragma once

#include "D3D12/D3D12RHI.hpp"
#include "Renderer/RenderPass.hpp"

namespace Luden
{
	class GeometryPass;
	class ShaderCompiler;
	class SceneCamera;

	struct SSAOParameters
	{
		DirectX::XMMATRIX Projection;

		uint32 OutputImageIndex;
		uint32 BaseColorIndex;
		uint32 NormalIndex;
		uint32 WorldPositionIndex;

		DirectX::XMFLOAT4 Samples[64];
		DirectX::XMFLOAT4 Noise[16];
	};

	class SSAO : public RenderPass
	{
	public:
		SSAO(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~SSAO();

		void Render(Frame& CurrentFrame, GeometryPass* pGBuffer, SceneCamera* pCamera);
		void Resize(uint32 Width, uint32 Height) override;

		void Release() override;

		D3D12RenderTexture RenderTarget;

		SSAOParameters Parameters{};

		D3D12ConstantBuffer* ConstantBuffer;

	private:
		D3D12RHI* m_D3D12RHI = nullptr;

		void CreatePipelines(ShaderCompiler* pShaderCompiler);

	};

} // namespace Luden

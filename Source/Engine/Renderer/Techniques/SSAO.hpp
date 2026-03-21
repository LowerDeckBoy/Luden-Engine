#pragma once

#include "D3D12/D3D12RHI.hpp"
#include "Renderer/RenderPass.hpp"

namespace Luden
{
	class GeometryPass;
	class ShaderCompiler;
	class AssetImporter;
	class SceneCamera;

	class SSAO : public RenderPass
	{
	public:
		SSAO(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~SSAO();

		void Render(Frame& CurrentFrame, GeometryPass* pGBuffer, uint32 NoiseImageIndex, SceneCamera* pCamera);
		void Resize(uint32 Width, uint32 Height) override;

		void Release() override;

		D3D12RenderTexture SSAORenderTarget;
		D3D12RenderTexture BlurRenderTarget;

		constexpr static uint32 KernelSize = 64;

		struct 
		{
			DirectX::XMMATRIX Projection;
			DirectX::XMMATRIX InvProjection;

			uint32 OutputImageIndex;
			uint32 NoiseIndex;
			uint32 NormalIndex;
			uint32 DepthIndex;

			float Radius	= 0.1f;
			float Bias		= 0.05f;
			float padding0	= 0.0f;
			float padding1	= 0.0f;

			DirectX::XMFLOAT4 Samples[KernelSize];
		} Parameters{};

		D3D12ConstantBuffer* ConstantBuffer;
		
		float BlurSharpness = 0.9f;
		bool bBlurSSAO = true;

	private:
		D3D12RHI* m_D3D12RHI = nullptr;

		D3D12Pipeline m_BlurPSO;

	};

} // namespace Luden

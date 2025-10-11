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

		D3D12RenderTexture RenderTarget;

		constexpr static uint32 KernelSize = 32;

		struct 
		{
			DirectX::XMMATRIX Projection;
			DirectX::XMMATRIX InvProjection;

			uint32 OutputImageIndex;
			uint32 NoiseIndex;
			uint32 NormalIndex;
			uint32 DepthIndex;

			float Radius	= 0.2f;
			float Power		= 5.0f;
			float Bias		= 0.1f;
			uint32 padding	= 0;

			DirectX::XMFLOAT4 Samples[KernelSize];
		} Parameters{};

		D3D12ConstantBuffer* ConstantBuffer;

		bool bBlurSSAO = false;

	private:
		D3D12RHI* m_D3D12RHI = nullptr;

		D3D12Pipeline m_BlurPSO;

	};

} // namespace Luden

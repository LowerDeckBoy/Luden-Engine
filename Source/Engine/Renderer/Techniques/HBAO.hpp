#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;
	class AssetImporter;

	class HBAO : public RenderPass
	{
	public:
		HBAO(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, AssetImporter* pAssetImporter, uint32 Width, uint32 Height);
		~HBAO();

		//void Render(Frame& CurrentFrame, GeometryPass* pGBuffer, uint32 NoiseImageIndex, SceneCamera* pCamera);
		void Resize(uint32 Width, uint32 Height) override;

		void Release() override;

		D3D12RenderTexture RenderTarget;

		//SSAOParameters Parameters{};

		D3D12ConstantBuffer* ConstantBuffer;

		D3D12Texture* NoiseTexture;

	private:
		D3D12RHI* m_D3D12RHI = nullptr;

		//void CreatePipelines(ShaderCompiler* pShaderCompiler);

	};

} // namespace Luden

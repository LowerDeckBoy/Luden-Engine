#pragma once

#include "D3D12/D3D12RHI.hpp"

namespace Luden
{
	class ShaderCompiler;

	//struct BloomConstants
	//{
	//	uint32 BaseColor;
	//	uint32 LightImage;
	//	uint32 SceneImage;
	//	uint32 pad;
	//};

	class Bloom
	{
	public:
		Bloom(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~Bloom();
	
		void Render(Frame& CurrentFrame, uint32 BaseColorImage, uint32 LightPassImageIndex, uint32 SceneImageIndex);
		void Resize(uint32 Width, uint32 Height);

		D3D12RenderTexture RenderTarget;

		D3D12Pipeline BloomPSO;
		D3D12Pipeline CombinePSO;

	private:
		D3D12RHI* m_D3D12RHI;

		void CreatePipelines(ShaderCompiler* pShaderCompiler);

	};
} // namespace Luden

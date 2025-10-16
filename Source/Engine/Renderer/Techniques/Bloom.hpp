#pragma once

#include "D3D12/D3D12RHI.hpp"
#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;

	/*
	struct BloomParameters
	{
		// Temp
		uint32	BaseColor;
		uint32	LightImage;
		uint32	SceneImage;
		float	Threshold	= 1.0f;
		float	Intensity	= 1.25f;
		float	Exposure	= 1.0f;
		// Temp
		float	Gamma		= 1.0f;
		// For Down and Up sampling textures.
		uint32	MipIndex;
	};
	*/

	class Bloom : public RenderPass
	{
	public:
		Bloom(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~Bloom();
	
		void Render(Frame& CurrentFrame, uint32 EmissiveImageIndex, uint32 LightPassImageIndex, uint32 SceneImageIndex);
		void Resize(uint32 Width, uint32 Height) override;

		void Combine(Frame& CurrentFrame, D3D12RenderTexture* pSceneImage, uint32 ImageIndex);

		void Release() override;

		D3D12RenderTexture RenderTarget;

		D3D12Pipeline BloomPSO;
		D3D12Pipeline DownsamplePSO;
		D3D12Pipeline UpsamplePSO;
		D3D12Pipeline CombinePSO;

		struct
		{
			uint32	EmissiveImage;
			uint32	LightImage;
			uint32	SceneImage;
			float	Threshold		= 1.0f;
			float	ThresholdKnee	= 0.5f;
			float	Intensity		= 1.0f;
			float	Gamma			= 1.0f;
			// For Down and Up sampling textures.
			uint32	MipIndex;
		} Parameters;

		std::vector<D3D12RenderTexture> DownsampleTextures;
		std::vector<D3D12RenderTexture> UpsampleTextures;

		static constexpr uint32 NumDownsamples	= 5;
		static constexpr uint32 NumUpsamples	= NumDownsamples + 1;

	private:
		//D3D12RHI* m_D3D12RHI;

		void CreatePipelines(ShaderCompiler* pShaderCompiler);

		void CreateTextures(uint32 Width, uint32 Height);

	};
} // namespace Luden

#pragma once

// https://github.com/PuddingCoke/Direct3D-12-toy-engine/blob/master/Engine/Shaders/FXAA/FXAA.hlsl

#include "../RenderPass.hpp"
#include "D3D12/D3D12RHI.hpp"

namespace Luden
{
	class ShaderCompiler;

	struct FXAAParameters
	{
		uint32	SceneImageIndex;
		uint32	OutputImageIndex;
		float	QualitySubpixel		= 8.0f;
		float	EdgeThreshold		= 1.0f / 8.0f;
		float	EdgeThresholdMin	= 1.0f / 128.0f;
	};

	class FXAA : public RenderPass
	{
	public:
		FXAA(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~FXAA();

		void Render(Frame& CurrentFrame, uint32 SceneImageIndex, uint32 OutputImageIndex);
		void Resize(uint32 Width, uint32 Height) override;

		void Release() override;

		D3D12RenderTexture RenderTarget;

		FXAAParameters Parameters;

	private:
		void CreatePipelines(ShaderCompiler* pShaderCompiler);

	};

} // namespace Luden

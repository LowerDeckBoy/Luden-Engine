#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;

	class FilmEffects : public RenderPass
	{
	public:
		FilmEffects(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~FilmEffects();

		struct
		{
			uint32 InputImageIndex;
			uint32 OutputImageIndex;
			uint32 EnableChromaticAberration;
			uint32 EnableLensDistortion;
			uint32 EnableFilmGrain;

			float LensDistortionIntensity = 1.0f;
		} Parameters;

		void Render(Frame& CurrentFrame, uint32 SceneImageIndex, uint32 Width, uint32 Height);

		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

		D3D12RenderTexture RenderTarget;

	private:

	};
} // namespace Luden

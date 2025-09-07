#pragma once

#include "D3D12/D3D12RHI.hpp"

namespace Luden
{
	class ShaderCompiler;

	enum TonemapMode : int32
	{
		None,
		Reinhard,
		GammaCorrection,
		Uncharted2,
		ACES,
		AGX,
		Hable
	};

	class Tonemapping
	{
	public:
		Tonemapping(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler);
		~Tonemapping();

		void Render(Frame& CurrentFrame, uint32 SceneImageIndex, uint32 Width, uint32 Height);

		float Exposure = 1.45f;
		int32 Mode = TonemapMode::ACES;

		double RenderTime = 0.0;

	private:
		D3D12Pipeline PSO;

	};
} // namespace Luden

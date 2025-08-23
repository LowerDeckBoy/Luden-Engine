#pragma once

#include "D3D12/D3D12RHI.hpp"

namespace Luden
{
	class ShaderCompiler;

	enum TonemapType : int32
	{
		None,
		Reinhard,
		GammaCorrection,
		Uncharted2,
		ACES,
		Hable
	};

	class Tonemapping
	{
	public:
		Tonemapping(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler);
		~Tonemapping();

		void Render(Frame& CurrentFrame, uint32 SceneImageIndex, uint32 Width, uint32 Height);

		float Exposure = 1.45f;
		int32 Type = TonemapType::ACES;

		double RenderTime = 0.0;

	private:
		D3D12Pipeline PSO;

	};
} // namespace Luden

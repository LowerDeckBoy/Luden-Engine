#pragma once

#include "D3D12/D3D12RHI.hpp"

namespace Luden
{
	struct ChromaticAberrationParameters
	{
		float View; // Camera
		float IndexOfRefraction;
	};

	// https://taylorpetrick.com/blog/post/dispersion-opengl
	class ChromaticAberration
	{
	public:
		ChromaticAberration();
		~ChromaticAberration();

		void Render(Frame& CurrentFrame, uint32 SceneImageIndex, uint32 NormalIndex);

		ChromaticAberrationParameters Parameters{};

	private:

		void CreatePipelines();

	};
} // namespace Luden

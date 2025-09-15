#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;
	class Scene;
	class SceneCamera;
	struct Frame;

	// Draw all Blend objects in forward shading.
	class Transparency : public RenderPass
	{
	public:
		Transparency(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler);
		~Transparency();

		void Render(Frame& CurrentFrame, uint32 SceneImage);

		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

	private:

	};
} // namespace Luden


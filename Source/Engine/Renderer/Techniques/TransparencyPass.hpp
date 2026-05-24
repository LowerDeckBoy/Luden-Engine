#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;
	class Scene;
	class SceneCamera;
	struct Frame;

	// Draw all Blend objects in forward shading.
	class TransparencyPass : public RenderPass
	{
	public:
		TransparencyPass(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler);
		~TransparencyPass();

		void Render(Frame& CurrentFrame, D3D12RenderTexture* pRenderTarget, D3D12Descriptor* pDepthStencil, Scene* pScene);

		// Does not write to it's own render target, so resizing isn't required.
		void Resize(uint32 /* Width */, uint32 /* Height */) override {}
		void Release() override;

	private:

	};
} // namespace Luden


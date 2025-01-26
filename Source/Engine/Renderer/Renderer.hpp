#pragma once

#include <Core/Core.hpp>

#include "D3D12/D3D12RHI.hpp"
#include "RenderPass.hpp"

namespace Luden
{
	struct SceneRenderTargets
	{
		uint32 GBuffer;
		// Final image
		//uint32 Scene;
		D3D12RenderTexture Scene;
	};
	 
	class Renderer
	{
	public:
		Renderer(D3D12RHI* pD3D12RHI);
		~Renderer();

		void Update();
		void Render();
		void Present(uint32 SyncInterval);

		static SceneRenderTargets SceneTextures;

		// - G-Buffer
		std::vector<RenderPass> RenderPasses;



	private:
		D3D12RHI* m_D3D12RHI;

	};
} // namespace Luden

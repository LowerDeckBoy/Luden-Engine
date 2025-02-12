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
		Renderer(Platform::Window* pParentWindow, D3D12RHI* pD3D12RHI);
		~Renderer();

		void BeginFrame();
		void EndFrame();

		void Update();
		void Render();
		void Present(uint32 SyncInterval);

		void Resize();

		static SceneRenderTargets SceneTextures;

		// - G-Buffer
		std::vector<RenderPass> RenderPasses;

		D3D12RHI* GetRHI() { return m_D3D12RHI; }

	private:
		D3D12RHI* m_D3D12RHI;
		Platform::Window* m_ParentWindow;

	};
} // namespace Luden

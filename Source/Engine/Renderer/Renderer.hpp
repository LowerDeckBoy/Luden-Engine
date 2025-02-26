#pragma once

#include "D3D12/D3D12RHI.hpp"
#include "RenderPass.hpp"
#include "Scene/Scene.hpp"
#include <Core/Core.hpp>

#include "RHI/Constants.hpp"

#include "Asset/ShaderCompiler.hpp"

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

		void Update(f64 DeltaTime);
		void Render(Scene* pScene);
		void Present(uint32 SyncInterval);

		void Resize();

		void Draw(Scene* pScene, Frame& CurrentFrame);

		static SceneRenderTargets SceneTextures;

		// - G-Buffer
		std::vector<RenderPass> RenderPasses;

		D3D12RHI* GetRHI() { return m_D3D12RHI; }

		void BuildPipelines();

		Scene* ActiveScene;

		void InitializeScene(Scene* pScene);

		SceneCamera* Camera;

	private:
		D3D12RHI* m_D3D12RHI;
		Platform::Window* m_ParentWindow;

		ShaderCompiler* m_ShaderCompiler;

		D3D12Shader BaseVS;
		D3D12Shader BasePS;

		D3D12RootSignature BaseRS;
		D3D12PipelineState BasePSO;

		D3D12Shader VertexVS;
		D3D12Shader VertexPS;

		D3D12RootSignature VertexRS;
		D3D12PipelineState VertexPSO;

		D3D12Shader MeshMS;
		D3D12Shader MeshPS;

		D3D12RootSignature MeshRS;
		D3D12PipelineState MeshPSO;

	};
} // namespace Luden

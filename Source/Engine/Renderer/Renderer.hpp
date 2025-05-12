#pragma once

#include "D3D12/D3D12RHI.hpp"
#include "RenderPass.hpp"
#include "Scene/Scene.hpp"
#include <Core/Core.hpp>
#include "RHI/Constants.hpp"
#include "Asset/ShaderCompiler.hpp"

#include "Techniques/GeometryPass.hpp"
#include "Techniques/LightPass.hpp"


namespace Luden
{
	struct SceneRenderTargets
	{
		// Final image
		D3D12RenderTexture Scene;

		D3D12Descriptor* ImageToDisplay;
	};

	struct SceneConstants
	{
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;

		DirectX::XMMATRIX InvView;
		DirectX::XMMATRIX InvProjection;

		std::array<DirectX::XMFLOAT4, 6> FrustumPlanes;

		f32 NearZ;
		f32 FarZ;

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

		//void Draw(Scene* pScene, Frame& CurrentFrame);

		// Draw given scene to Geometry Buffer.
		void DrawScene(Scene* pScene, Frame& CurrentFrame);

		void BuildScene(Scene* pScene);

		static SceneRenderTargets SceneTextures;

		// - G-Buffer
		std::vector<RenderPass> RenderPasses;

		D3D12RHI* GetRHI() { return m_D3D12RHI; }

		void BuildPipelines();

		Scene* ActiveScene;

		void ReleaseActiveScene();

		SceneCamera* Camera;

		// TODO:
		// Clean these up
		bool bRequestCleanup = false;
		bool bRequestSceneLoad = false;
		Filepath SceneToLoad;
		
		uint64 CulledVertices = 0;

		// Test
		SceneConstants cbScene{};

		// Render Passes
		GeometryPass*	GBuffer;
		LightPass*		LightingPass;
	
		// Test
		D3D12BVH* RaytracingBVH;
		void InitializeRaytracingResources();

	private:
		D3D12RHI* m_D3D12RHI;
		Platform::Window* m_ParentWindow;

		ShaderCompiler* m_ShaderCompiler;

		D3D12Shader VertexVS;
		D3D12Shader VertexPS;

		D3D12RootSignature VertexRS;
		D3D12PipelineState VertexPSO;

		D3D12Shader MeshAS;
		D3D12Shader MeshMS;
		D3D12Shader MeshPS;

		D3D12RootSignature MeshRS;
		D3D12PipelineState MeshPSO;

		D3D12Shader MeshCullAS;
		D3D12Shader MeshCullMS;
		D3D12Shader MeshCullPS;

		D3D12RootSignature MeshCullRS;
		D3D12PipelineState MeshCullPSO;

	};
} // namespace Luden

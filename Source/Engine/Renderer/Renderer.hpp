#pragma once

#include "D3D12/D3D12RHI.hpp"
#include "RenderPass.hpp"
#include "Scene/Scene.hpp"
#include <Core/Core.hpp>
#include "RHI/Constants.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "Asset/AssetImporter.hpp"

#include "Techniques/GeometryPass.hpp"
#include "Techniques/LightPass.hpp"
#include "Techniques/Bloom.hpp"
#include "Techniques/FXAA.hpp"
#include "Techniques/SSAO.hpp"
#include "Techniques/SSR.hpp"
#include "Techniques/Tonemapping.hpp"
#include "Techniques/Skybox.hpp"

// Test
#include "D3D12/D3D12StateObject.hpp"

namespace Luden
{
	struct SceneRenderTargets
	{
		// Final scene image.
		D3D12RenderTexture Scene;
		// Immediate image for all post processes.
		//D3D12RenderTexture PostProcess;
	};

	// TODO:
	enum class ERenderLayer
	{
		Opaque,
		Transparent,
		Sky
	};

	class Renderer
	{
	public:
		Renderer(Platform::Window* pParentWindow, D3D12RHI* pD3D12RHI, AssetImporter* pAssetImporter);
		~Renderer();

		void BeginFrame();
		void EndFrame();

		void Update(f64 DeltaTime);
		void Render(Scene* pScene);
		void Present(uint32 SyncInterval);

		void Resize();

		void BuildScene(Scene* pScene);

		static SceneRenderTargets SceneTextures;

		D3D12RHI* GetRHI() { return m_D3D12RHI; }

		Scene* ActiveScene = nullptr;

		void ReleaseActiveScene();

		SceneCamera* Camera = nullptr;

		// TODO:
		// Clean these up
		bool bRequestCleanup = false;
		bool bRequestSceneLoad = false;
		Filepath SceneToLoad;

		// Render Passes
		GeometryPass*	GBuffer;
		LightPass*		LightingPass;
		Bloom*			BloomPass;
		FXAA*			FXAAPass;
		Tonemapping*	TonemappingPass;
		SSR*			SSRPass;
		SSAO*			SSAOPass;
		Skybox*			SkyboxPass;
		ProceduralSky*	ProceduralSkyPass;

		double PresentRenderTime = 0.0;
		double UpdateRenderTime = 0.0;

		D3D12Texture* NoiseTexture = nullptr;
	
	private:
		D3D12RHI* m_D3D12RHI = nullptr;
		Platform::Window* m_ParentWindow = nullptr;

		ShaderCompiler* m_ShaderCompiler;
		AssetImporter* m_AssetImporter;

	public:
		// Test
		D3D12BVH* RaytracingBVH = nullptr;
		D3D12StateObject* RaytracingPSO = nullptr;
		void InitializeRaytracingResources();
		D3D12Shader* RayGenShader;
		D3D12Shader* MissShader;
		D3D12Shader* ClosestHitShader;
		D3D12ShaderBindingTable* RaytracingShaderTable;

		D3D12RootSignature* RaytracingRS;
		D3D12Texture* RaytracingOutput;
		void DispatchRayTracing(Frame& CurrentFrame);

	};
} // namespace Luden

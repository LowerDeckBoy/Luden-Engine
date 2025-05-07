#include <Asset/AssetImporter.hpp>
#include "Renderer.hpp"
#include "D3D12/D3D12Utility.hpp"
#include <Core/Logger.hpp>
#include <Core/Math/Math.hpp>

namespace Luden
{
	SceneRenderTargets Renderer::SceneTextures = {};

	Renderer::Renderer(Platform::Window* pParentWindow, D3D12RHI* pD3D12RHI)
		: m_ParentWindow(pParentWindow),
		m_D3D12RHI(pD3D12RHI)
	{
		SceneTextures.Scene.Create(m_D3D12RHI->Device,
			static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Width),
			static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Height),
			m_D3D12RHI->SwapChain->GetSwapChainFormat(),
			DefaultClearColor,
			"Scene Output Render Texture");

		m_ShaderCompiler = new ShaderCompiler();

		Camera = new SceneCamera(pParentWindow);

		VertexVS = m_ShaderCompiler->CompileVS("../../Shaders/Vertex.hlsl", true);
		VertexPS = m_ShaderCompiler->CompilePS("../../Shaders/Vertex.hlsl", false);

		MeshAS = m_ShaderCompiler->CompileAS("../../Shaders/MS.hlsl", true);
		MeshMS = m_ShaderCompiler->CompileMS("../../Shaders/MS.hlsl", true);
		MeshPS = m_ShaderCompiler->CompilePS("../../Shaders/MS.hlsl", false);

		MeshCullAS = m_ShaderCompiler->CompileAS("../../Shaders/MSCulled.hlsl", true);
		MeshCullMS = m_ShaderCompiler->CompileMS("../../Shaders/MSCulled.hlsl", true);
		MeshCullPS = m_ShaderCompiler->CompilePS("../../Shaders/MSCulled.hlsl", false);

		GBuffer		 = new GeometryPass(pD3D12RHI, pParentWindow->Width, pParentWindow->Height);
		LightingPass = new LightPass(pD3D12RHI, GBuffer, pParentWindow->Width, pParentWindow->Height);

		BuildPipelines();

		//SceneTextures.ImageToDisplay = &SceneTextures.Scene.ShaderResourceHandle;
		SceneTextures.ImageToDisplay = &GBuffer->BaseColor.ShaderResourceHandle;

	}

	Renderer::~Renderer()
	{
		//delete RaytracingBVH;

		delete LightingPass;
		delete GBuffer;

		SceneTextures.Scene.Release();
		D3D12UploadContext::Release();
		
	}

	void Renderer::BeginFrame()
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);
		
		VERIFY_D3D12_RESULT(frame->GraphicsCommandList->Open());

		frame->GraphicsCommandList->SetDescriptorHeap(m_D3D12RHI->Device->ShaderResourceHeap);
		frame->GraphicsCommandList->SetViewport(&m_D3D12RHI->SwapChain->GetSwapChainViewport());

	}

	void Renderer::EndFrame()
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);
		auto& backbuffer = m_D3D12RHI->SwapChain->BackBuffers.at(BackBufferIndex);

		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_PRESENT);

		m_D3D12RHI->GraphicsQueue->Execute({ frame->GraphicsCommandList });
		
	}

	void Renderer::Update(f64 DeltaTime)
	{
		Camera->Tick(DeltaTime);

		//Frustum.Update(Camera->Projection *);
		//cbScene.FrustumPlanes

	}

	void Renderer::Render(Scene* pScene)
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);
		auto& backbuffer = m_D3D12RHI->SwapChain->BackBuffers.at(BackBufferIndex);

		auto& depthStencilView = m_D3D12RHI->SceneDepthBuffer->DepthStencilHandle;

		frame->GraphicsCommandList->ClearDepthStencilView(depthStencilView);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetCpuStartHandle(), BackBufferIndex, m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetDescriptorIncrementSize());
		//frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthStencilView.CpuHandle);
		frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_RENDER_TARGET);
		frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &SceneTextures.Scene.RenderTargetHandle.CpuHandle, FALSE, &depthStencilView.CpuHandle);
		//constexpr float clearColor[4] = { 0.5f, 0.2f, 0.7f, 1.0f };
		frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(SceneTextures.Scene.RenderTargetHandle.CpuHandle, DefaultClearColor.data(), 0, nullptr);

		frame->GraphicsCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		GBuffer->Render(*frame, [&](){ DrawScene(pScene, *frame); });
		LightingPass->Render(ActiveScene, *frame);
		//Draw(pScene, *frame);

		frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_GENERIC_READ);

		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(rtvHandle, DefaultClearColor.data(), 0, nullptr);

	}

	void Renderer::Present(uint32 SyncInterval)
	{
		m_D3D12RHI->Present(SyncInterval);
		
	}

	void Renderer::Resize()
	{
		m_D3D12RHI->Wait();
		m_D3D12RHI->Flush();

		for (uint32 i = 0; i < Config::Get().NumBackBuffers; ++i)
		{
			m_D3D12RHI->FrameSync.CurrentValues.at(i) = m_D3D12RHI->FrameSync.CurrentValues.at(BackBufferIndex);

			m_D3D12RHI->Frames.at(i).GraphicsCommandList->Open();

		}

		m_ParentWindow->Resize();
		m_D3D12RHI->SwapChain->Resize(m_ParentWindow->Width, m_ParentWindow->Height);
		m_D3D12RHI->SceneDepthBuffer->Resize(m_ParentWindow->Width, m_ParentWindow->Height);

		SceneTextures.Scene.Resize(m_ParentWindow->Width, m_ParentWindow->Height);
		GBuffer->Resize(m_ParentWindow->Width, m_ParentWindow->Height);
		LightingPass->Resize(m_ParentWindow->Width, m_ParentWindow->Height);
		
		std::vector<D3D12CommandList*> lists;
		for (auto& frame : m_D3D12RHI->Frames)
		{
			lists.emplace_back(frame.GraphicsCommandList);
		}

		m_D3D12RHI->GraphicsQueue->Execute(lists);
		m_D3D12RHI->Wait();


		Camera->Resize();

	}

	/*
	void Renderer::Draw(Scene* pScene, Frame& CurrentFrame)
	{
		if (!pScene || pScene->Models.empty())
		{
			return;
		}
		
		CulledVertices = 0;

		auto* device = m_D3D12RHI->Device;
		
		if (Config::Get().bMeshShading)
		{
			CurrentFrame.GraphicsCommandList->SetRootSignature(&MeshCullRS);
			CurrentFrame.GraphicsCommandList->SetPipelineState(&MeshCullPSO);

			for (auto& model : pScene->Models)
			{
				auto& transform = model->GetComponent<ecs::TransformComponent>();
				transform.Update();
				
				auto* constantBuffer = device->ConstantBuffers.at(model->ConstantBuffer);
				CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);
				
				for (auto& mesh : model->Meshes)
				{
					if (!Camera->IsInsideFrustum(mesh.BoundingBox))
					{
						CulledVertices += mesh.NumVertices;
						//continue;
					}

					mesh.Transform.Update();
					model->cbObjectTransforms.WVP	= DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix * Camera->GetViewProjection());
					model->cbObjectTransforms.World	= DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix);
					constantBuffer->Update(&model->cbObjectTransforms);

					uint32 vertexBuffer				= device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
					uint32 meshletBuffer			= device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
					uint32 meshletVerticesBuffer	= device->Buffers.at(mesh.MeshletVerticesBuffer)->ShaderResourceView.Index;
					uint32 meshletTrianglesBuffer	= device->Buffers.at(mesh.MeshletTrianglesBuffer)->ShaderResourceView.Index;
					uint32 meshletBoundsBuffer		= device->Buffers.at(mesh.MeshletBoundsBuffer)->ShaderResourceView.Index;

					auto& material = model->Materials.at(mesh.MaterialId);

					struct
					{
						uint32 vertex;
						uint32 meshlet;
						uint32 meshletVertices;
						uint32 meshletTriangles;
						uint32 meshletBounds;
						uint32 bDrawMeshlets;
						uint32 bAlphaMask;
					} buffers
					{
						.vertex = vertexBuffer,
						.meshlet = meshletBuffer,
						.meshletVertices = meshletVerticesBuffer,
						.meshletTriangles = meshletTrianglesBuffer,
						.meshletBounds = meshletBoundsBuffer,
						.bDrawMeshlets = (uint32)Config::Get().bDrawMeshlets,
						.bAlphaMask = (uint32)Config::Get().bAlphaMask
					};

					CurrentFrame.GraphicsCommandList->PushConstants(1, 7, &buffers);
					CurrentFrame.GraphicsCommandList->PushConstants(2, 20, &material);

					//CurrentFrame.GraphicsCommandList->DispatchMesh(mesh.NumMeshlets, 1, 1);
					CurrentFrame.GraphicsCommandList->DispatchMesh(ROUND_UP(mesh.NumMeshlets / 32), 1, 1);
				}
			}
		}
		else
		{
			CurrentFrame.GraphicsCommandList->SetRootSignature(&VertexRS);
			CurrentFrame.GraphicsCommandList->SetPipelineState(&VertexPSO);
			
			for (auto& model : pScene->Models)
			{
				auto& transform = model->GetComponent<ecs::TransformComponent>();
				transform.Update();
		
				auto* constantBuffer = device->ConstantBuffers.at(model->ConstantBuffer);
				CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);
		
				for (auto& mesh : model->Meshes)
				{
					//if (!Camera->IsInsideFrustum(mesh.BoundingBox))
					//{
					//	CulledVertices += mesh.NumVertices;
					//	//continue;
					//}

					//mesh.Transform.Update();
					model->cbObjectTransforms.WVP	= DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix * Camera->GetViewProjection());
					model->cbObjectTransforms.World	= DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix);
					constantBuffer->Update(&model->cbObjectTransforms);

					auto* vertexBuffer = device->Buffers.at(mesh.VertexBuffer);
		
					CurrentFrame.GraphicsCommandList->PushConstants(1, 1, &vertexBuffer->ShaderResourceView.Index);
		
					if (mesh.NumIndices != 0)
					{
						CurrentFrame.GraphicsCommandList->SetIndexBuffer(mesh.IndexBufferView);
						CurrentFrame.GraphicsCommandList->DrawIndexed(mesh.NumIndices, 0, 0);
					}
				}
			}
		}

	}
	*/

	void Renderer::DrawScene(Scene* pScene, Frame& CurrentFrame)
	{
		if (!pScene || pScene->Models.empty())
		{
			return;
		}

		auto device = m_D3D12RHI->Device;

		for (auto& model : pScene->Models)
		{
			auto& transform = model->GetComponent<ecs::TransformComponent>();
			transform.Update();

			auto* constantBuffer = device->ConstantBuffers.at(model->ConstantBuffer);

			model->cbObjectTransforms.WVP = DirectX::XMMatrixTranspose(transform.WorldMatrix * Camera->GetViewProjection());
			model->cbObjectTransforms.World = DirectX::XMMatrixTranspose(transform.WorldMatrix);
			constantBuffer->Update(&model->cbObjectTransforms);

			CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);

			for (auto& mesh : model->Meshes)
			{
				uint32 vertexBuffer				= device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
				uint32 meshletBuffer			= device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
				uint32 meshletVerticesBuffer	= device->Buffers.at(mesh.MeshletVerticesBuffer)->ShaderResourceView.Index;
				uint32 meshletTrianglesBuffer	= device->Buffers.at(mesh.MeshletTrianglesBuffer)->ShaderResourceView.Index;
				uint32 meshletBoundsBuffer		= device->Buffers.at(mesh.MeshletBoundsBuffer)->ShaderResourceView.Index;

				auto& material = model->Materials.at(mesh.MaterialId);

				struct
				{
					uint32 vertex;
					uint32 meshlet;
					uint32 meshletVertices;
					uint32 meshletTriangles;
					uint32 meshletBounds;
					uint32 bDrawMeshlets;
					uint32 bAlphaMask;
				} buffers
				{
					.vertex				= vertexBuffer,
					.meshlet			= meshletBuffer,
					.meshletVertices	= meshletVerticesBuffer,
					.meshletTriangles	= meshletTrianglesBuffer,
					.meshletBounds		= meshletBoundsBuffer,
					.bDrawMeshlets		= (uint32)Config::Get().bDrawMeshlets,
					.bAlphaMask			= (uint32)Config::Get().bAlphaMask
				};

				CurrentFrame.GraphicsCommandList->PushConstants(1, 7, &buffers);
				CurrentFrame.GraphicsCommandList->PushConstants(2, 20, &material);

				//CurrentFrame.GraphicsCommandList->DispatchMesh(mesh.NumMeshlets, 1, 1);
				CurrentFrame.GraphicsCommandList->DispatchMesh(ROUND_UP(mesh.NumMeshlets / 32), 1, 1);
			}
		}
	}

	void Renderer::BuildScene(Scene* pScene)
	{
		assert(pScene != nullptr);
		
		ActiveScene = pScene;

		if (pScene->Models.empty())
		{
			return;
		}

		//m_D3D12RHI->MeshCommandSignature = new D3D12CommandSignature();

		//std::vector<FDispatchMeshCommand> drawCommands;

		for (auto& model : pScene->Models)
		{
			// Initialize resources.
			model->Create(m_D3D12RHI->Device);
			// Gather indirect arguments.
			/*
			for (usize meshIdx = 0; meshIdx < model.Meshes.size(); ++meshIdx)
			{
				m_D3D12RHI->MeshCommandSignature->AddDispatchMeshCommand();

				FDispatchMeshCommand command{};
				command.Argument.ThreadGroupCountX = model.Meshes.at(meshIdx).NumMeshlets;
				command.Argument.ThreadGroupCountY = 1;
				command.Argument.ThreadGroupCountZ = 1;

				drawCommands.push_back(command);
			}
			*/
		}
		//https://kidswithsticks.com/how-to-create-rain-in-unreal-engine-4-niagara/
		

		//InitializeRaytracingResources();

			


	}

	void Renderer::BuildPipelines()
	{
		// Vertex
		{
			VERIFY_D3D12_RESULT(VertexRS.BuildFromShader(m_D3D12RHI->Device, &VertexVS, PipelineType::Graphics));

			D3D12PipelineStateBuilder builder(m_D3D12RHI->Device);
			builder.SetRootSignature(&VertexRS);
			builder.SetVertexShader(&VertexVS);
			builder.SetPixelShader(&VertexPS);
			builder.EnableDepth(true);
			builder.SetCullMode(D3D12_CULL_MODE_BACK);
			builder.SetRenderTargetFormats({ m_D3D12RHI->SwapChain->GetSwapChainFormat() });
			
			VERIFY_D3D12_RESULT(builder.Build(VertexPSO));
		}

		// Mesh
		{
			VERIFY_D3D12_RESULT(MeshRS.BuildFromShader(m_D3D12RHI->Device, &MeshMS, PipelineType::Graphics));

			D3D12MeshPipelineStateBuilder builder(m_D3D12RHI->Device);
			builder.SetRootSignature(&MeshRS);
			builder.SetAmplificationShader(&MeshAS);
			builder.SetMeshShader(&MeshMS);
			builder.SetPixelShader(&MeshPS);
			builder.EnableDepth(true);
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
			builder.SetRenderTargetFormats({ m_D3D12RHI->SwapChain->GetSwapChainFormat() });

			VERIFY_D3D12_RESULT(builder.Build(MeshPSO));
		}

		// MeshCull
		{
			VERIFY_D3D12_RESULT(MeshCullRS.BuildFromShader(m_D3D12RHI->Device, &MeshCullMS, PipelineType::Graphics));

			D3D12MeshPipelineStateBuilder builder(m_D3D12RHI->Device);
			builder.SetRootSignature(&MeshCullRS);
			builder.SetAmplificationShader(&MeshCullAS);
			builder.SetMeshShader(&MeshCullMS);
			builder.SetPixelShader(&MeshCullPS);
			builder.EnableDepth(true);
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
			builder.SetRenderTargetFormats({ m_D3D12RHI->SwapChain->GetSwapChainFormat() });

			VERIFY_D3D12_RESULT(builder.Build(MeshCullPSO));
		}

		// GBuffer
		{
			GBuffer->Pipeline.Amplification = m_ShaderCompiler->CompileAS("../../Shaders/Mesh/GBuffer_MS.hlsl", false);
			GBuffer->Pipeline.Mesh			= m_ShaderCompiler->CompileMS("../../Shaders/Mesh/GBuffer_MS.hlsl", true);
			GBuffer->Pipeline.Pixel			= m_ShaderCompiler->CompilePS("../../Shaders/Mesh/GBuffer_MS.hlsl", false);
			
			VERIFY_D3D12_RESULT(GBuffer->Pipeline.RootSignature.BuildFromShader(m_D3D12RHI->Device, &GBuffer->Pipeline.Mesh, PipelineType::Graphics));

			D3D12MeshPipelineStateBuilder builder(m_D3D12RHI->Device);
			builder.SetRootSignature(&GBuffer->Pipeline.RootSignature);
			builder.SetAmplificationShader(&GBuffer->Pipeline.Amplification);
			builder.SetMeshShader(&GBuffer->Pipeline.Mesh);
			builder.SetPixelShader(&GBuffer->Pipeline.Pixel);
			builder.EnableDepth(true);
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
			builder.SetAlphaOpaqueMode(0);
			builder.SetRenderTargetFormats({ 
				GBuffer->BaseColor.GetFormat(),
				GBuffer->Normal.GetFormat(),
				GBuffer->MetallicRoughness.GetFormat(),
				GBuffer->Emissive.GetFormat()
			});

			VERIFY_D3D12_RESULT(builder.Build(GBuffer->Pipeline.PipelineState));
		}

		// LightPass
		{
			LightingPass->Pipeline.Vertex = m_ShaderCompiler->CompileVS("../../Shaders/Deferred/Deferred.hlsl", false);
			LightingPass->Pipeline.Pixel  = m_ShaderCompiler->CompilePS("../../Shaders/Deferred/Deferred.hlsl", true);

			VERIFY_D3D12_RESULT(LightingPass->Pipeline.RootSignature.BuildFromShader(m_D3D12RHI->Device, &LightingPass->Pipeline.Pixel, PipelineType::Graphics));

			D3D12PipelineStateBuilder builder(m_D3D12RHI->Device);
			builder.EnableDepth(false);
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
			builder.SetVertexShader(&LightingPass->Pipeline.Vertex);
			builder.SetPixelShader(&LightingPass->Pipeline.Pixel);
			builder.SetRenderTargetFormats({ LightingPass->RenderTexture.GetFormat() });

			VERIFY_D3D12_RESULT(builder.Build(LightingPass->Pipeline.PipelineState));
		}

	}

	/*
	bool Renderer::AddModel(AssetImporter* pAssetImporter, Filepath Path)
	{
		pAssetImporter->Device = m_D3D12RHI->Device;

		Model model{};
		auto importStartTime = std::chrono::high_resolution_clock::now();

		ActiveScene->GetWorld()->CreateEntity(&model);

		model.AddComponent<ecs::NameComponent>(File::GetFilename(Path));
		model.AddComponent<ecs::TransformComponent>();

		if (!pAssetImporter->ImportStaticMesh(Path, model))
		{	
			// LOG
			LOG_ERROR("Failed to add model!");
			return false;
		}

		auto importEndTime = std::chrono::high_resolution_clock::now();
		LOG_DEBUG("{0} import time: {1}", File::GetFilename(Path), std::chrono::duration<f64>(importEndTime - importStartTime));

		model.Create(m_D3D12RHI->Device);

		ActiveScene->Models.push_back(std::make_unique<Model>(model));

		return true;

	}*/
	
	void Renderer::ReleaseActiveScene()
	{
		m_D3D12RHI->Wait();

		auto unloadStartTime = std::chrono::high_resolution_clock::now();

		for (auto& frame : m_D3D12RHI->Frames)
		{
			frame.GraphicsCommandList->Open();
		}

		for (auto buffer : m_D3D12RHI->Device->Buffers)
		{
			delete buffer;
		}

		for (auto buffer : m_D3D12RHI->Device->ConstantBuffers)
		{
			delete buffer;
		}
		
		m_D3D12RHI->Device->ConstantBuffers.clear(); 
		m_D3D12RHI->Device->ConstantBuffers.shrink_to_fit();
		m_D3D12RHI->Device->Buffers.clear();
		m_D3D12RHI->Device->Buffers.shrink_to_fit();
		
		ActiveScene->Release();

		std::vector<D3D12CommandList*> lists;
		for (auto& frame : m_D3D12RHI->Frames)
		{
			lists.emplace_back(frame.GraphicsCommandList);
		}

		//m_D3D12RHI->Device->ShaderResourceHeap->Reset();
		m_D3D12RHI->GraphicsQueue->Execute(lists);
		m_D3D12RHI->Wait();
		
		GBuffer->Initialize(m_D3D12RHI, m_ParentWindow->Width, m_ParentWindow->Height);

		auto unloadEndTime = std::chrono::high_resolution_clock::now();
		LOG_DEBUG("Scene: {0} unloaded. Unload time: {1}.", ActiveScene->Name, std::chrono::duration<f64>(unloadEndTime - unloadStartTime));

	}

	void Renderer::HandleRequests()
	{
		if (bRequestCleanup)
		{
			ReleaseActiveScene();
			bRequestCleanup = false;
		}

		//if (bRequestSceneLoad)
		//{
		//	bRequestSceneLoad = false;
		//	MainScene = nullptr;
		//	MainScene = new Scene();
		//	SceneSerializer::Load(&Importer, MainScene, m_Renderer->SceneToLoad);
		//	InitializeScene(MainScene);
		//	ActiveScene = MainScene;
		//	SceneToLoad = "";
		//	GetRHI()->Wait();
		//	//continue;
		//}
	}

	void Renderer::InitializeRaytracingResources()
	{
		RaytracingBVH = new D3D12BVH(m_D3D12RHI);

		for (auto& model : ActiveScene->Models)
		{
			RaytracingBVH->AddBLAS(model.get());
		}

		RaytracingBVH->CreateTLAS();

	}

} // namespace Luden

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
		SceneTextures.Scene.Create(m_D3D12RHI->Device, {
			.Usage  = TextureUsageFlag::RenderTarget,
			.Data  = nullptr,
			.Width  = static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Width),
			.Height = static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Height),
			.Format = m_D3D12RHI->SwapChain->GetSwapChainFormat(),
			},
			"Scene Output Render Texture");

		m_ShaderCompiler = new ShaderCompiler();

		Camera = new SceneCamera(pParentWindow);

		VertexVS = m_ShaderCompiler->CompileVS("../../Shaders/Vertex.hlsl", true);
		VertexPS = m_ShaderCompiler->CompilePS("../../Shaders/Vertex.hlsl", false);

		MeshAS = m_ShaderCompiler->CompileAS("../../Shaders/MS.hlsl", true);
		MeshMS = m_ShaderCompiler->CompileMS("../../Shaders/MS.hlsl", true);
		MeshPS = m_ShaderCompiler->CompilePS("../../Shaders/MS.hlsl", false);

		BuildPipelines();

	}

	Renderer::~Renderer()
	{
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
		constexpr float clearColor[4] = { 0.5f, 0.2f, 0.7f, 1.0f };
		frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(SceneTextures.Scene.RenderTargetHandle.CpuHandle, clearColor, 0, nullptr);

		frame->GraphicsCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Draw(pScene, *frame);

		frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_GENERIC_READ);

		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

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
		
		std::vector<D3D12CommandList*> lists;
		for (auto& frame : m_D3D12RHI->Frames)
		{
			lists.emplace_back(frame.GraphicsCommandList);
		}

		m_D3D12RHI->GraphicsQueue->Execute(lists);
		m_D3D12RHI->Wait();

		Camera->Resize();

	}

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
			CurrentFrame.GraphicsCommandList->SetRootSignature(&MeshRS);
			CurrentFrame.GraphicsCommandList->SetPipelineState(&MeshPSO);

			for (auto& model : pScene->Models)
			{
				auto& transform = model.GetComponent<ecs::TransformComponent>();
				transform.Update();
				
				auto* constantBuffer = device->ConstantBuffers.at(model.ConstantBuffer);
				CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);
				
				for (auto& mesh : model.Meshes)
				{
					mesh.Transform.Update();
					model.cbObjectTransforms.WVP	= DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix * Camera->GetViewProjection());
					model.cbObjectTransforms.World	= DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix);
					constantBuffer->Update(&model.cbObjectTransforms);

					uint32 vertexBuffer				= device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
					uint32 meshletBuffer			= device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
					uint32 meshletVerticesBuffer	= device->Buffers.at(mesh.MeshletVerticesBuffer)->ShaderResourceView.Index;
					uint32 meshletTrianglesBuffer	= device->Buffers.at(mesh.MeshletTrianglesBuffer)->ShaderResourceView.Index;

					struct
					{
						uint32 vertex;
						uint32 meshlet;
						uint32 meshletVertices;
						uint32 meshletTriangles;
						uint32 bDrawMeshlets;
					} buffers
					{
						.vertex = vertexBuffer,
						.meshlet = meshletBuffer,
						.meshletVertices = meshletVerticesBuffer,
						.meshletTriangles = meshletTrianglesBuffer,
						.bDrawMeshlets = (uint32)Config::Get().bDrawMeshlets,
					};

					CurrentFrame.GraphicsCommandList->PushConstants(1, 5, &buffers);

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
				auto& transform = model.GetComponent<ecs::TransformComponent>();
				transform.Update();
		
				auto* constantBuffer = device->ConstantBuffers.at(model.ConstantBuffer);
				CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);
		
				for (auto& mesh : model.Meshes)
				{
					mesh.Transform.Update();
					model.cbObjectTransforms.WVP	= DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix * Camera->GetViewProjection());
					model.cbObjectTransforms.World	= DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix);
					constantBuffer->Update(&model.cbObjectTransforms);
					

					//DirectX::XMStoreFloat4x4(&mat, Camera->GetViewProjection());
					//DirectX::XMStoreFloat4x4(&mat, transform.WorldMatrix);
					
					//DirectX::XMStoreFloat4x4(&mat, mesh.Transform.WorldMatrix * Camera->GetViewProjection());
					//DirectX::XMFLOAT4X4 mat;
					//DirectX::XMStoreFloat4x4(&mat, model.cbObjectTransforms.World * Camera->GetViewProjection());
					//if (!Camera->IsInFrustrum(mesh.BoundingBox, mat))
					//{
					//	CulledVertices += mesh.NumVertices;
					//	continue;
					//}

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
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
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

	}

	void Renderer::InitializeScene(Scene* pScene)
	{
		for (auto& model : pScene->Models)
		{
			model.Create(m_D3D12RHI->Device);
		}

		//D3D12UploadContext::Upload();

	}

	bool Renderer::AddModel(AssetImporter* pAssetImporter, Filepath Path)
	{
		Model model{};
		
		auto importStartTime = std::chrono::high_resolution_clock::now();

		ActiveScene->GetWorld()->CreateEntity(&model);

		model.AddComponent<ecs::NameComponent>(File::GetFilename(Path));
		model.AddComponent<ecs::TransformComponent>();

		if (!pAssetImporter->ImportStaticMesh(Path, model))
		{	
			// LOG
			return false;
		}

		auto importEndTime = std::chrono::high_resolution_clock::now();
		LOG_DEBUG("{0} import time: {1}", File::GetFilename(Path), std::chrono::duration<f64>(importEndTime - importStartTime));

		model.Create(m_D3D12RHI->Device);

		ActiveScene->Models.push_back(model);

		return true;

	}

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
		
		for (auto& model : ActiveScene->Models)
		{
			model.Release();
		}

		ActiveScene->Models.clear();

		std::vector<D3D12CommandList*> lists;
		for (auto& frame : m_D3D12RHI->Frames)
		{
			lists.emplace_back(frame.GraphicsCommandList);
		}

		m_D3D12RHI->Device->ShaderResourceHeap->Reset();
		m_D3D12RHI->GraphicsQueue->Execute(lists);
		m_D3D12RHI->Wait();
		
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

} // namespace Luden

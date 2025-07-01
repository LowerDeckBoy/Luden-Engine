#include <Asset/AssetImporter.hpp>
#include "Renderer.hpp"
#include "D3D12/D3D12Utility.hpp"
#include <Core/Logger.hpp>
#include <Core/Math/Math.hpp>
#include "ECS/Components/LightComponent.hpp"

namespace Luden
{
	SceneRenderTargets Renderer::SceneTextures = {};

	Renderer::Renderer(Platform::Window* pParentWindow, D3D12RHI* pD3D12RHI)
		: m_ParentWindow(pParentWindow)
	{
		m_D3D12RHI = pD3D12RHI;

		SceneTextures.Scene.Create(m_D3D12RHI->Device,
			static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Width),
			static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Height),
			m_D3D12RHI->SwapChain->GetSwapChainFormat(),
			DefaultClearColor,
			"Scene Output Render Texture");

		m_ShaderCompiler = new ShaderCompiler();

		Camera = new SceneCamera(pParentWindow);

		GBuffer = new GeometryPass(pD3D12RHI, m_ShaderCompiler, pParentWindow->Width, pParentWindow->Height);
		LightingPass = new LightPass(pD3D12RHI, m_ShaderCompiler, GBuffer, pParentWindow->Width, pParentWindow->Height);

		//SceneTextures.ImageToDisplay = &SceneTextures.Scene.ShaderResourceHandle;
		SceneTextures.ImageToDisplay = &GBuffer->BaseColor.ShaderResourceHandle;

	}

	Renderer::~Renderer()
	{
		m_D3D12RHI->Flush();

		if (RaytracingBVH)
		{
			delete RaytracingBVH;
			delete RaytracingOutput;
			delete RaytracingOutputSR;
			delete RaytracingRS;
			delete RaytracingPSO;
			delete RaytracingShaderTable;
			delete RayGenShader;
			delete MissShader;
			delete ClosestHitShader;
		}
		
		delete LightingPass;
		delete GBuffer;

		SceneTextures.Scene.Release();
		D3D12UploadContext::Release();

	}

	void Renderer::BeginFrame()
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);

		if (!frame->GraphicsCommandList->IsOpen())
		{
			frame->GraphicsCommandList->Open();
		}

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

		ActiveScene->Consts.Position  = Camera->Position;
		ActiveScene->Consts.Planes[0] = Camera->Frustum.Planes[0]; // Right
		ActiveScene->Consts.Planes[1] = Camera->Frustum.Planes[1]; // Left
		ActiveScene->Consts.Planes[2] = Camera->Frustum.Planes[2]; // Top
		ActiveScene->Consts.Planes[3] = Camera->Frustum.Planes[3]; // Bottom
		ActiveScene->Consts.Planes[4] = Camera->Frustum.Planes[4]; // Far
		ActiveScene->Consts.Planes[5] = Camera->Frustum.Planes[5]; // Near
		
	}

	void Renderer::Render(Scene* /* pScene */)
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);
		auto& backbuffer = m_D3D12RHI->SwapChain->BackBuffers.at(BackBufferIndex);

		auto& depthStencilView = m_D3D12RHI->SceneDepthBuffer->DepthStencilHandle;

		frame->GraphicsCommandList->ClearDepthStencilView(depthStencilView);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetCpuStartHandle(), BackBufferIndex, m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetDescriptorIncrementSize());
		

		//frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthStencilView.CpuHandle);
		//frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(SceneTextures.Scene.RenderTargetHandle.CpuHandle, DefaultClearColor.data(), 0, nullptr);

		frame->GraphicsCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (!Config::Get().bRaytracing)
		{
			GBuffer->Render(ActiveScene, Camera, *frame);
			//frame->GraphicsCommandList->ResourceTransition(m_D3D12RHI->SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
			LightingPass->Render(ActiveScene, *frame, Camera);
			//frame->GraphicsCommandList->ResourceTransition(m_D3D12RHI->SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			//LightingPass->Render(ActiveScene, *frame);
		}
		else
		{
			//frame->GraphicsCommandList->ResourceTransition(RaytracingOutputSR, D3D12_RESOURCE_STATE_GENERIC_READ);
			DispatchRayTracing(*frame);

			frame->GraphicsCommandList->ResourceTransition({
				{ RaytracingOutput, D3D12_RESOURCE_STATE_COPY_SOURCE },
				{ &GBuffer->BaseColor, D3D12_RESOURCE_STATE_COPY_DEST }
				//{ RaytracingOutputSR, D3D12_RESOURCE_STATE_COPY_DEST }
				});
			//frame->GraphicsCommandList->CopyResource(RaytracingOutput, RaytracingOutputSR);
			frame->GraphicsCommandList->CopyResource(RaytracingOutput, &GBuffer->BaseColor);
			frame->GraphicsCommandList->ResourceTransition({
				{ RaytracingOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
				{ &GBuffer->BaseColor, D3D12_RESOURCE_STATE_GENERIC_READ   }
				//{ RaytracingOutputSR, D3D12_RESOURCE_STATE_GENERIC_READ   }
			});

		}

		//frame->GraphicsCommandList->ResourcesTransition({
		//	{ &SceneTextures.Scene, D3D12_RESOURCE_STATE_GENERIC_READ },
		//	{ &backbuffer,			D3D12_RESOURCE_STATE_RENDER_TARGET } });
		
		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(rtvHandle, DefaultClearColor.data(), 0, nullptr);

		// Debug usage only
		//if (Config::Get().bHideEditor)
		//{
		//	frame->GraphicsCommandList->ResourcesTransition({
		//		{ &GBuffer->BaseColor,	D3D12_RESOURCE_STATE_COPY_SOURCE },
		//		{ &backbuffer,			D3D12_RESOURCE_STATE_COPY_DEST } });
		//	//frame->GraphicsCommandList->CopyResource(&GBuffer->BaseColor, &backbuffer);
		//	frame->GraphicsCommandList->CopyResource(RaytracingOutputSR, &backbuffer);
		//	frame->GraphicsCommandList->ResourcesTransition({
		//		{ &GBuffer->BaseColor,	D3D12_RESOURCE_STATE_GENERIC_READ },
		//		{ &backbuffer,			D3D12_RESOURCE_STATE_RENDER_TARGET } });
		//}

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

		if (RaytracingBVH != nullptr)
		{
			RaytracingOutput->Resize(m_ParentWindow->Width, m_ParentWindow->Height);
			RaytracingOutputSR->Resize(m_ParentWindow->Width, m_ParentWindow->Height);
		}

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

			// Draw all opaque objects.
			for (auto& mesh : model->OpaqueMeshes)
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
					uint32 pad = 0;
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

				//CurrentFrame.GraphicsCommandList->DispatchMesh(ROUND_UP(mesh.NumMeshlets / 32), 1, 1);
			}

			if (model->BlendMeshes.empty())
			{
				return;
			}

			// Draw all transparent objects.
			for (auto& mesh : model->BlendMeshes)
			{
				uint32 vertexBuffer = device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
				uint32 meshletBuffer = device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
				uint32 meshletVerticesBuffer = device->Buffers.at(mesh.MeshletVerticesBuffer)->ShaderResourceView.Index;
				uint32 meshletTrianglesBuffer = device->Buffers.at(mesh.MeshletTrianglesBuffer)->ShaderResourceView.Index;
				uint32 meshletBoundsBuffer = device->Buffers.at(mesh.MeshletBoundsBuffer)->ShaderResourceView.Index;

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

	void Renderer::BuildScene(Scene* pScene)
	{
		assert(pScene != nullptr);
		
		ActiveScene = pScene;

		if (pScene->Models.empty())
		{
			return;
		}

		if (m_D3D12RHI->MeshCommandSignature != nullptr)
		{
			m_D3D12RHI->MeshCommandSignature->Release();
			m_D3D12RHI->MeshCommandSignature = nullptr;
		}

		m_D3D12RHI->MeshCommandSignature = new D3D12CommandSignature(m_D3D12RHI->Device);
		
		std::vector<FDispatchMeshCommand> drawCommands;

		for (auto& model : pScene->Models)
		{
			// Initialize resources.
			model->Create(m_D3D12RHI->Device);
			
			// Gather indirect arguments.
			/*
			for (usize meshIdx = 0; meshIdx < model->Meshes.size(); ++meshIdx)
			{
				m_D3D12RHI->MeshCommandSignature->AddDispatchMeshCommand();
			
				auto& mesh = model->Meshes.at(meshIdx);
			
				FDispatchMeshCommand command{};
				command.Argument.ThreadGroupCountX	= mesh.NumMeshlets;
				command.Argument.ThreadGroupCountY	= 1;
				command.Argument.ThreadGroupCountZ	= 1;
			
				command.MeshletBufferIndex			= mesh.MeshletsBuffer;
				command.MeshletVerticesIndex		= mesh.MeshletVerticesBuffer;
				command.MeshletTrianglesIndex		= mesh.MeshletTrianglesBuffer;
				command.MeshletBoundsBufferIndex	= mesh.MeshletBoundsBuffer;
			
				drawCommands.push_back(command);
			}
			*/
		}
		
		// To finish:
		//BufferDesc desc{};
		//desc.Data			= drawCommands.data();
		//desc.NumElements	= static_cast<uint32>(drawCommands.size());
		//desc.Stride			= sizeof(FDispatchMeshCommand);
		//desc.Size			= desc.NumElements * desc.Stride;
		//desc.BufferUsage	= BufferUsageFlag::IndirectArgument;
		//desc.Name			= "D3D12 Command Signature Indirect Buffer";
		//m_D3D12RHI->MeshCommandSignature->CreateCommandsBuffer(desc);

		//InitializeRaytracingResources();

		pScene->SceneDataBuffer = new D3D12ConstantBuffer(GetRHI()->Device, &pScene->SceneData, sizeof(pScene->SceneData));

		// Material buffer

		if (pScene->MaterialBuffer != nullptr)
		{
			delete pScene->MaterialBuffer;
			pScene->MaterialBuffer = nullptr;
		}

		uint64 alignedSize = Math::Align<uint64>(sizeof(Material) * 4096, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		pScene->MaterialBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{ 
			.BufferUsage = BufferUsageFlag::Storage,
			.Data = pScene->Materials.data(),
			.NumElements = 4096,
			.Stride = sizeof(Material),
			.Size = alignedSize,
			.bBindless = true,
			.Name = "Scene Material Buffer" });

		usize mapSize = static_cast<usize>(pScene->Materials.size() * sizeof(Material));
		VERIFY_D3D12_RESULT(pScene->MaterialBuffer->GetHandle()->Map(0, nullptr, &pScene->MaterialBuffer->GetBufferDesc().Data));
		std::memcpy(pScene->MaterialBuffer->GetBufferDesc().Data, pScene->Materials.data(), mapSize);
		// Should I keep it persistent?
		//pScene->MaterialBuffer->GetHandle()->Unmap(0, nullptr);

		// Lighting buffer
		if (pScene->LightBuffer != nullptr)
		{
			delete pScene->LightBuffer;
			pScene->LightBuffer = nullptr;
		}

		alignedSize = Math::Align<uint64>(sizeof(ecs::PointLightComponent) * 128, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		pScene->LightBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::Storage,
			.Data = pScene->PointLights.data(),
			.NumElements = 128,
			.Stride = sizeof(ecs::PointLightComponent),
			.Size = alignedSize,
			.bBindless = true,
			.Name = "Scene Lighting Buffer" });

		mapSize = static_cast<usize>(pScene->PointLights.size() * sizeof(ecs::PointLightComponent));
		VERIFY_D3D12_RESULT(pScene->LightBuffer->GetHandle()->Map(0, nullptr, &pScene->LightBuffer->GetBufferDesc().Data));
		std::memcpy(pScene->LightBuffer->GetBufferDesc().Data, pScene->PointLights.data(), mapSize);

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
		
		ActiveScene->Release();
		
		std::vector<D3D12CommandList*> lists;
		for (auto& frame : m_D3D12RHI->Frames)
		{
			lists.emplace_back(frame.GraphicsCommandList);
		}

		m_D3D12RHI->GraphicsQueue->Execute(lists);
		m_D3D12RHI->Wait();
		
		auto unloadEndTime = std::chrono::high_resolution_clock::now();
		LOG_DEBUG("Scene: {0} unloaded. Unload time: {1}.", ActiveScene->Name, std::chrono::duration<f64>(unloadEndTime - unloadStartTime));

	}

	void Renderer::InitializeRaytracingResources()
	{
		RaytracingBVH = new D3D12BVH(m_D3D12RHI);

		for (auto& model : ActiveScene->Models)
		{
			RaytracingBVH->AddBLAS(model.get());
		}

		RaytracingBVH->CreateTLAS();

		TextureDesc textureDesc{};
		textureDesc.Width = m_ParentWindow->Width;
		textureDesc.Height = m_ParentWindow->Height;
		textureDesc.Format = m_D3D12RHI->SwapChain->GetSwapChainFormat();
		textureDesc.Usage = TextureUsageFlag::UnorderedAccess;
		RaytracingOutput = new D3D12Texture(m_D3D12RHI->Device, textureDesc);
		RaytracingOutput->SetDebugName("D3D12 Raytracing Output Texture");
		RaytracingOutputSR = new D3D12Texture(m_D3D12RHI->Device, textureDesc);
		RaytracingOutputSR->SetDebugName("D3D12 Raytracing Output Texture SR");
	
		RayGenShader		= new D3D12Shader(m_ShaderCompiler->CompileLib("../../Shaders/Raytracing/Base/RayGen.hlsl", false, "RayGen"));
		MissShader			= new D3D12Shader(m_ShaderCompiler->CompileLib("../../Shaders/Raytracing/Base/Miss.hlsl", false, "Miss"));
		ClosestHitShader	= new D3D12Shader(m_ShaderCompiler->CompileLib("../../Shaders/Raytracing/Base/ClosestHit.hlsl", false, "ClosestHit"));

		RaytracingRS = new D3D12RootSignature();
		//RaytracingRS->AddCBV(0, 1);
		RaytracingRS->AddConstants(52, 0, 1);
		RaytracingRS->AddSRV(0, 1);
		RaytracingRS->AddStaticSampler(0, 0, D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_ALWAYS);
		//RaytracingRS->SetGlobal();
		VERIFY_D3D12_RESULT(RaytracingRS->Build(m_D3D12RHI->Device, PipelineType::Compute));

		D3D12StateObjectBuilder builder;
		builder.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
		builder.AddRayGen(RayGenShader, { L"RayGen" });
		builder.AddMiss(MissShader, { L"Miss" });
		builder.AddClosestHit(ClosestHitShader, { L"ClosestHit" });
		builder.SetPayloadSize(16);
		builder.SetMaxRayRecursion(1);
		//builder.SetGlobalRootSignature(RaytracingRS, { L"RayGen" });
		
		FHitGroup hitGroup{};
		hitGroup.Name = "HitGroup";
		hitGroup.ClosestHitName = "ClosestHit";
		hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

		builder.AddHitGroup(hitGroup);

		builder.SetGlobalRootSignature(RaytracingRS, { L"RayGen", L"Miss", L"HitGroup" });
		RaytracingPSO = new D3D12StateObject();
		builder.Build(m_D3D12RHI->Device, *RaytracingPSO);
		
		RaytracingShaderTable = new D3D12ShaderBindingTable();
		
		struct globalArgs
		{
			void* pDescriptorHeap;
		} args {
			.pDescriptorHeap = reinterpret_cast<uint64*>(m_D3D12RHI->Device->ShaderResourceHeap->GetGpuStartHandlePtr())
		};

		FShaderTableRecord raygenRecord(FShaderIdentifier(RaytracingPSO->GetProperties()->GetShaderIdentifier(L"RayGen")));
		//FShaderTableRecord raygenRecord(FShaderIdentifier(RaytracingPSO->GetProperties()->GetShaderIdentifier(L"RayGen")), &args, sizeof(globalArgs));
		RaytracingShaderTable->RayGenTable.AddRecord(raygenRecord);

		FShaderTableRecord missRecord(FShaderIdentifier(RaytracingPSO->GetProperties()->GetShaderIdentifier(L"Miss")));
		RaytracingShaderTable->MissTable.AddRecord(missRecord);

		FShaderTableRecord hitRecord(FShaderIdentifier(RaytracingPSO->GetProperties()->GetShaderIdentifier(L"HitGroup")));
		RaytracingShaderTable->HitTable.AddRecord(hitRecord);

		RaytracingShaderTable->Create(m_D3D12RHI->Device);
		RaytracingShaderTable->m_StorageBuffer->SetDebugName("D3D12 STB Storage");

		

	}

	void Renderer::DispatchRayTracing(Frame& CurrentFrame)
	{
		auto* commandList = CurrentFrame.GraphicsCommandList;

		//commandList->SetDescriptorHeap(m_D3D12RHI->Device->ShaderResourceHeap);
		commandList->SetRootSignature(RaytracingRS);
		commandList->SetPipelineState1(RaytracingPSO);
		commandList->GetHandle()->SetComputeRootShaderResourceView(1, RaytracingBVH->TLAS->AccelerationStructure->GetGpuAddress());

		struct constData
		{
			DirectX::XMMATRIX View;
			DirectX::XMMATRIX Projection;
			DirectX::XMMATRIX ViewProjection;
			DirectX::XMFLOAT3 CameraPosition;
			uint32 RaytracingImage;
		} consts{
			.View				= DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, Camera->GetView())),
			.Projection			= DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, Camera->GetProjection())),
			.ViewProjection		= DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, Camera->GetViewProjection())),
			.CameraPosition		= Camera->Position,
			.RaytracingImage	= RaytracingOutput->UnorderedAccessHandle.Index
			//.RaytracingImage = RaytracingOutput->ShaderResourceHandle.Index
		};
		commandList->GetHandle()->SetComputeRoot32BitConstants(0, 52, &consts, 0);

		D3D12_DISPATCH_RAYS_DESC desc{};
		desc.Width  = static_cast<uint32>(RaytracingOutput->GetDesc().Width);
		desc.Height = RaytracingOutput->GetDesc().Height;

		desc.RayGenerationShaderRecord.StartAddress = RaytracingShaderTable->m_StorageBuffer->GetGpuAddress() + RaytracingShaderTable->RayGenOffset;
		desc.RayGenerationShaderRecord.SizeInBytes = RaytracingShaderTable->RayGenTable.GetSizeInBytes();

		desc.MissShaderTable.StartAddress	= desc.RayGenerationShaderRecord.StartAddress + RaytracingShaderTable->MissOffset;
		desc.MissShaderTable.SizeInBytes	= RaytracingShaderTable->MissTable.GetSizeInBytes();
		desc.MissShaderTable.StrideInBytes	= RaytracingShaderTable->MissTable.GetStride();

		desc.HitGroupTable.StartAddress		= desc.RayGenerationShaderRecord.StartAddress + RaytracingShaderTable->HitOffset;
		desc.HitGroupTable.SizeInBytes		= RaytracingShaderTable->HitTable.GetSizeInBytes();
		desc.HitGroupTable.StrideInBytes	= RaytracingShaderTable->HitTable.GetStride();

		desc.Depth = 1;

		CurrentFrame.GraphicsCommandList->GetHandle()->DispatchRays(&desc);
		
	}

} // namespace Luden

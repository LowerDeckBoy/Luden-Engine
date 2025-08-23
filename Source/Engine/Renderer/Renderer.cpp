#include <Asset/AssetImporter.hpp>
#include "Renderer.hpp"
#include "D3D12/D3D12Utility.hpp"
#include <Core/Logging/Logger.hpp>
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

		GBuffer			= new GeometryPass(pD3D12RHI, m_ShaderCompiler, pParentWindow->Width, pParentWindow->Height);
		LightingPass	= new LightPass(pD3D12RHI, m_ShaderCompiler, GBuffer, pParentWindow->Width, pParentWindow->Height);

		BloomPass		= new Bloom(pD3D12RHI, m_ShaderCompiler, pParentWindow->Width, pParentWindow->Height);
		FXAAPass		= new FXAA(pD3D12RHI, m_ShaderCompiler, pParentWindow->Width, pParentWindow->Height);
		TonemappingPass = new Tonemapping(pD3D12RHI, m_ShaderCompiler);
		SSAOPass		= new SSAO(pD3D12RHI, m_ShaderCompiler, pParentWindow->Width, pParentWindow->Height);

	}

	Renderer::~Renderer()
	{
		m_D3D12RHI->Flush();

		if (RaytracingBVH)
		{
			delete RaytracingBVH;
			delete RaytracingOutput;
			delete RaytracingRS;
			delete RaytracingPSO;
			delete RaytracingShaderTable;
			delete RayGenShader;
			delete MissShader;
			delete ClosestHitShader;
		}

		delete FXAAPass;
		delete TonemappingPass;
		delete SSAOPass;
		delete BloomPass;
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

		m_D3D12RHI->GraphicsQueue->Execute({ frame->GraphicsCommandList, frame->ComputeCommandList });

	}

	void Renderer::Update(f64 DeltaTime)
	{
		Camera->Tick(DeltaTime);

		ActiveScene->UpdateSceneBufferData(Camera);

		for (auto& model : ActiveScene->Models)
		{
			auto& transformComponent = model->GetComponent<ecs::TransformComponent>();

			auto& transform	= ActiveScene->Transforms.at(model->TransformID);

			if (transformComponent.bDirty)
			{
				transformComponent.Update();
			}

			transform.WVP			= transformComponent.WorldMatrix * Camera->GetViewProjection();
			transform.World			= transformComponent.WorldMatrix;
			transform.PreviousWorld = transformComponent.PreviousPosition * Camera->GetViewProjection();
		}

		std::memcpy(ActiveScene->TransformsBuffer->GetBufferDesc().Data, ActiveScene->Transforms.data(), (ActiveScene->Transforms.size() * sizeof(ecs::ObjectTransforms)));

	}

	void Renderer::Render(Scene* /* pScene */)
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);
		auto& backbuffer = m_D3D12RHI->SwapChain->BackBuffers.at(BackBufferIndex);
		auto* commandList = frame->GraphicsCommandList;

		auto& depthStencilView = m_D3D12RHI->SceneDepthBuffer->DepthStencilHandle;

		commandList->ClearDepthStencilView(depthStencilView);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetCpuStartHandle(), BackBufferIndex, m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetDescriptorIncrementSize());

		commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (!Config::Get().bRaytracing)
		{
			// G-Buffer
			if (!Config::Get().bDrawIndirect)
			{
				GBuffer->Render(ActiveScene, Camera, *frame);
			}
			else
			{
				GBuffer->RenderIndirect(ActiveScene, Camera, *frame);
			}
			// Not used for now.

			if (Config::Get().bEnableSSAO)
			{
				commandList->ResourceTransition(m_D3D12RHI->SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
				SSAOPass->Render(*frame, GBuffer, Camera);
				commandList->ResourceTransition(m_D3D12RHI->SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}


			// Open ComputeCommandList before dispatching Post-Processes and set DescriptorHeap once.
			if (!frame->ComputeCommandList->IsOpen())
			{
				frame->ComputeCommandList->Open();
			}

			frame->ComputeCommandList->SetDescriptorHeap(m_D3D12RHI->Device->ShaderResourceHeap);

			// Light Pass
			if (!Config::Get().bLightPassCompute)
			{
				LightingPass->Render(ActiveScene, *frame, Camera);
			}
			else
			{
				LightingPass->RenderCompute(ActiveScene, *frame, Camera);
			}

			// Post-Processes
			if (Config::Get().bEnablePostProcess)
			{
				frame->ComputeCommandList->ResourceTransition({
					{ &LightingPass->RenderTexture,	D3D12_RESOURCE_STATE_COPY_SOURCE },
					{ &SceneTextures.Scene,			D3D12_RESOURCE_STATE_COPY_DEST }
					});
				frame->ComputeCommandList->CopyResource(&LightingPass->RenderTexture, &SceneTextures.Scene);
				frame->ComputeCommandList->ResourceTransition({
					{ &LightingPass->RenderTexture,	D3D12_RESOURCE_STATE_GENERIC_READ },
					{ &SceneTextures.Scene,			D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
					});

				if (Config::Get().bEnableFXAA)
				{
					FXAAPass->Render(*frame, LightingPass->RenderTexture.ShaderResourceHandle.Index, SceneTextures.Scene.ShaderResourceHandle.Index);
				}

				if (Config::Get().bEnableBloom)
				{
					BloomPass->Render(*frame, GBuffer->Emissive.ShaderResourceHandle.Index, LightingPass->RenderTexture.ShaderResourceHandle.Index, BloomPass->RenderTarget.ShaderResourceHandle.Index);
					BloomPass->Combine(*frame, &SceneTextures.Scene, GBuffer->Emissive.ShaderResourceHandle.Index);
				}

				if (Config::Get().bEnableTonemapping)
				{
					TonemappingPass->Render(*frame, SceneTextures.Scene.ShaderResourceHandle.Index, m_ParentWindow->Width, m_ParentWindow->Height);
				}

				frame->ComputeCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_GENERIC_READ);
			}
		}
		else
		{
			commandList->ResourceTransition(RaytracingOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			DispatchRayTracing(*frame);
			commandList->ResourceTransition(RaytracingOutput, D3D12_RESOURCE_STATE_GENERIC_READ);

			commandList->ResourceTransition({
				{ &SceneTextures.Scene, D3D12_RESOURCE_STATE_COPY_DEST },
				{ RaytracingOutput,		D3D12_RESOURCE_STATE_COPY_SOURCE } });
			commandList->CopyResource(RaytracingOutput, &SceneTextures.Scene);
			commandList->ResourceTransition({
				{ &SceneTextures.Scene, D3D12_RESOURCE_STATE_GENERIC_READ },
				{ RaytracingOutput,		D3D12_RESOURCE_STATE_UNORDERED_ACCESS } });
		}

		commandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
		commandList->GetHandleRaw()->ClearRenderTargetView(rtvHandle, DefaultClearColor.data(), 0, nullptr);

		// Debug use only
		if (Config::Get().bHideEditor)
		{
			commandList->ResourceTransition({
				{ &GBuffer->BaseColor,	D3D12_RESOURCE_STATE_COPY_SOURCE },
				{ &backbuffer,			D3D12_RESOURCE_STATE_COPY_DEST } });
			commandList->CopyResource(&SceneTextures.Scene, &backbuffer);
			commandList->ResourceTransition({
				{ &GBuffer->BaseColor,	D3D12_RESOURCE_STATE_GENERIC_READ },
				{ &backbuffer,			D3D12_RESOURCE_STATE_RENDER_TARGET } });
		}

	}

	void Renderer::Present(uint32 SyncInterval)
	{
		auto presentBeginTime = Time::GetTimestamp();
		m_D3D12RHI->Present(SyncInterval);
		PresentRenderTime = Time::GetDurationInMiliseconds(presentBeginTime);
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

		const uint32 width  = m_ParentWindow->Width;
		const uint32 height = m_ParentWindow->Height;

		m_D3D12RHI->SwapChain->Resize(width, height);
		m_D3D12RHI->SceneDepthBuffer->Resize(width, height, 1.0f);

		SceneTextures.Scene.Resize(width, height);
		GBuffer->Resize(width, height);
		LightingPass->Resize(width, height);
		BloomPass->Resize(width, height);
		FXAAPass->Resize(width, height);
		SSAOPass->Resize(width, height);

		if (RaytracingBVH != nullptr)
		{
			RaytracingOutput->Resize(width, height);
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

	void Renderer::BuildScene(Scene* pScene)
	{
		assert(pScene != nullptr);
		
		ActiveScene = pScene;

		if (pScene->Models.empty())
		{
			return;
		}

		pScene->SceneDataBuffer = new D3D12ConstantBuffer(GetRHI()->Device, &pScene->SceneData, sizeof(pScene->SceneData));

		// Transforms buffer
		if (pScene->TransformsBuffer != nullptr)
		{
			delete pScene->TransformsBuffer;
			pScene->TransformsBuffer = nullptr;
		}

		uint64 alignedSize = Math::Align<uint64>(sizeof(ecs::ObjectTransforms) * 8192, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		pScene->TransformsBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::Storage,
			.Data = pScene->Transforms.data(),
			.NumElements = 8192,
			.Stride = sizeof(ecs::ObjectTransforms),
			.Size = alignedSize,
			.bBindless = true,
			.Name = "Scene Transforms Buffer" });

		VERIFY_D3D12_RESULT(pScene->TransformsBuffer->GetHandle()->Map(0, nullptr, &pScene->TransformsBuffer->GetBufferDesc().Data));
		std::memcpy(pScene->TransformsBuffer->GetBufferDesc().Data, pScene->Transforms.data(), (pScene->Transforms.size() * sizeof(ecs::ObjectTransforms)));

		// Material buffer
		if (pScene->MaterialBuffer != nullptr)
		{
			delete pScene->MaterialBuffer;
			pScene->MaterialBuffer = nullptr;
		}

		alignedSize = Math::Align<uint64>(sizeof(Material) * 4096, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

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

		// Point Lighting buffer
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
			.Name = "Scene Point Lighting Buffer" });

		VERIFY_D3D12_RESULT(pScene->LightBuffer->GetHandle()->Map(0, nullptr, &pScene->LightBuffer->GetBufferDesc().Data));

		// Spot Lighting buffer
		if (pScene->SpotLightBuffer != nullptr)
		{
			delete pScene->SpotLightBuffer;
			pScene->SpotLightBuffer = nullptr;
		}

		alignedSize = Math::Align<uint64>(sizeof(ecs::SpotLightComponent) * 128, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		pScene->SpotLightBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::Storage,
			.Data = pScene->SpotLights.data(),
			.NumElements = 128,
			.Stride = sizeof(ecs::SpotLightComponent),
			.Size = alignedSize,
			.bBindless = true,
			.Name = "Scene Spot Lighting Buffer" });

		VERIFY_D3D12_RESULT(pScene->SpotLightBuffer->GetHandle()->Map(0, nullptr, &pScene->SpotLightBuffer->GetBufferDesc().Data));


		pScene->CreateEntity(pScene->SkyLight);
		pScene->SkyLight.AddComponent<ecs::NameComponent>("Directional Light");
		pScene->SkyLight.AddComponent<ecs::DirectionalLightComponent>();

		/*
		*/
		if (GBuffer->IndirectSignature != nullptr)
		{
			GBuffer->IndirectSignature->Release();
			GBuffer->IndirectSignature = nullptr;
		}

		GBuffer->IndirectSignature = new D3D12CommandSignature(m_D3D12RHI->Device);
		std::vector<FDispatchMeshCommand> drawCommands;
		//GBuffer->IndirectSignature->AddConstantsCommand(14, 1);
		
		for (auto& model : pScene->Models)
		{
			// Initialize resources.
			model->Create(m_D3D12RHI->Device);

			// Gather indirect arguments.
			for (usize meshIdx = 0; meshIdx < model->Meshes.size(); ++meshIdx)
			{
				auto& mesh = model->Meshes.at(meshIdx);
			
				FDispatchMeshCommand command{};
				command.Argument.ThreadGroupCountX	= mesh.NumMeshlets;
				command.Argument.ThreadGroupCountY	= 1;
				command.Argument.ThreadGroupCountZ	= 1;
			
				

				command.VertexBufferIndex			= mesh.VertexBuffer;
				command.MeshletBufferIndex			= mesh.MeshletsBuffer;
				command.MeshletVerticesIndex		= mesh.MeshletVerticesBuffer;
				command.MeshletTrianglesIndex		= mesh.MeshletTrianglesBuffer;
				command.MeshletBoundsBufferIndex	= mesh.MeshletBoundsBuffer;
			
				command.TransformsBufferIndex		= pScene->TransformsBuffer->ShaderResourceView.Index;
				command.MaterialsBufferIndex		= pScene->MaterialBuffer->ShaderResourceView.Index;
				command.TransformID					= model->TransformID;
				command.MaterialID					= mesh.MaterialID;

				//GBuffer->IndirectSignature->AddConstantsCommand(14, 1);
			GBuffer->IndirectSignature->AddConstantsCommand(14, 1);
				drawCommands.push_back(command);
			}
		}
		GBuffer->IndirectSignature->AddDispatchMeshCommand();

		BufferDesc desc{};
		desc.Data			= drawCommands.data();
		desc.NumElements	= static_cast<uint32>(drawCommands.size());
		desc.Stride			= sizeof(FDispatchMeshCommand);
		desc.Size			= static_cast<uint64>(desc.NumElements * desc.Stride);
		desc.BufferUsage	= BufferUsageFlag::IndirectArgument;
		desc.Name			= "D3D12 Command Signature Indirect Buffer";
		
		//VERIFY_D3D12_RESULT(GBuffer->IndirectSignature->Build(m_D3D12RHI->Device, &GBuffer->IndirectPipelineState.RootSignature));
		//GBuffer->IndirectSignature->CreateCommandsBuffer(desc);

		//InitializeRaytracingResources();

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
		textureDesc.Width	= m_ParentWindow->Width;
		textureDesc.Height	= m_ParentWindow->Height;
		textureDesc.Format	= m_D3D12RHI->SwapChain->GetSwapChainFormat();
		textureDesc.Usage	= TextureUsageFlag::UnorderedAccess;
		
		RaytracingOutput	= new D3D12Texture(m_D3D12RHI->Device, textureDesc);
		RaytracingOutput->Subresource.RowPitch = textureDesc.Width;
		RaytracingOutput->Subresource.SlicePitch = 4 * RaytracingOutput->Subresource.RowPitch;
		RaytracingOutput->SetDebugName("D3D12 Raytracing Output Texture");
		
		D3D12UploadContext::UploadTexture(RaytracingOutput);
		D3D12UploadContext::Upload();

		RayGenShader		= new D3D12Shader(m_ShaderCompiler->CompileLib("../../Shaders/Raytracing/Base/RayGen.hlsl",		false, "RayGen"));
		MissShader			= new D3D12Shader(m_ShaderCompiler->CompileLib("../../Shaders/Raytracing/Base/Miss.hlsl",		false, "Miss"));
		ClosestHitShader	= new D3D12Shader(m_ShaderCompiler->CompileLib("../../Shaders/Raytracing/Base/ClosestHit.hlsl", false, "ClosestHit"));

		RaytracingRS = new D3D12RootSignature();
		RaytracingRS->AddConstants(54, 0, 0);
		RaytracingRS->AddSRV(0, 0);
		RaytracingRS->AddStaticSampler(0, 0, D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_ALWAYS);
		VERIFY_D3D12_RESULT(RaytracingRS->Build(m_D3D12RHI->Device, PipelineType::Compute));

		D3D12StateObjectBuilder builder;
		builder.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
		builder.AddRayGen(RayGenShader, { L"RayGen" });
		builder.AddMiss(MissShader, { L"Miss" });
		builder.AddClosestHit(ClosestHitShader, { L"ClosestHit" });
		builder.SetPayloadSize(16);
		builder.SetMaxRayRecursion(0);
		//builder.SetGlobalRootSignature(RaytracingRS, { L"RayGen" });
		
		FHitGroup hitGroup{};
		hitGroup.Name			= "HitGroup";
		hitGroup.Type			= D3D12_HIT_GROUP_TYPE_TRIANGLES;
		hitGroup.ClosestHitName = "ClosestHit";

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

		//FShaderTableRecord raygenRecord(FShaderIdentifier(RaytracingPSO->GetProperties()->GetShaderIdentifier(L"RayGen")));
		FShaderTableRecord raygenRecord(FShaderIdentifier(RaytracingPSO->GetProperties()->GetShaderIdentifier(L"RayGen")), &args, sizeof(globalArgs));
		//RaytracingShaderTable->RayGenTable.AddRecord(raygenRecord);

		FShaderTableRecord missRecord(FShaderIdentifier(RaytracingPSO->GetProperties()->GetShaderIdentifier(L"Miss")));
		RaytracingShaderTable->MissTable.AddRecord(missRecord);

		FShaderTableRecord hitRecord(FShaderIdentifier(RaytracingPSO->GetProperties()->GetShaderIdentifier(L"HitGroup")));
		RaytracingShaderTable->HitTable.AddRecord(hitRecord);

		RaytracingShaderTable->Create(m_D3D12RHI->Device);
		RaytracingShaderTable->m_StorageBuffer->SetDebugName("D3D12 SBT Storage");

	}

	void Renderer::DispatchRayTracing(Frame& CurrentFrame)
	{
		auto* commandList = CurrentFrame.GraphicsCommandList;

		commandList->SetRootSignature(RaytracingRS);
		commandList->SetPipelineState1(RaytracingPSO);

		struct constData
		{
			DirectX::XMMATRIX View;
			DirectX::XMMATRIX Projection;
			DirectX::XMMATRIX ViewProjection;
			DirectX::XMFLOAT3 CameraPosition;
			uint32 padding = 0;
			uint32 RaytracingImage;
			uint32 RaytracingTopLevel;
		} consts{
			.View				= Camera->GetView(),
			.Projection			= Camera->GetProjection(),
			.ViewProjection		= DirectX::XMMatrixTranspose(Camera->GetViewProjection()),
			//.ViewProjection		= DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, Camera->GetViewProjection())),
			.CameraPosition		= Camera->Position,
			.RaytracingImage	= RaytracingOutput->UnorderedAccessHandle.Index,
			.RaytracingTopLevel = RaytracingBVH->TLAS->AccelerationStructure->ShaderResourceView.Index

		};
		commandList->GetHandle()->SetComputeRoot32BitConstants(0, 54, &consts, 0);
		//commandList->GetHandle()->SetComputeRootShaderResourceView(1, RaytracingBVH->TLAS->AccelerationStructure->GetGpuAddress());
		//commandList->GetHandle()->SetComputeRootUnorderedAccessView(2, RaytracingOutput->GetGpuAddress());

		D3D12_DISPATCH_RAYS_DESC desc{};
		desc.Width  = static_cast<uint32>(RaytracingOutput->GetDesc().Width);
		desc.Height = RaytracingOutput->GetDesc().Height;

		
		desc.RayGenerationShaderRecord.StartAddress = RaytracingShaderTable->m_StorageBuffer->GetGpuAddress() + RaytracingShaderTable->RayGenOffset;
		desc.RayGenerationShaderRecord.SizeInBytes  = RaytracingShaderTable->RayGenTable.GetSizeInBytes();

		desc.MissShaderTable.StartAddress	= desc.RayGenerationShaderRecord.StartAddress + RaytracingShaderTable->MissOffset;
		desc.MissShaderTable.SizeInBytes	= RaytracingShaderTable->MissTable.GetSizeInBytes();
		desc.MissShaderTable.StrideInBytes	= RaytracingShaderTable->MissTable.GetStride();

		desc.HitGroupTable.StartAddress		= desc.RayGenerationShaderRecord.StartAddress + RaytracingShaderTable->HitOffset;
		desc.HitGroupTable.SizeInBytes		= RaytracingShaderTable->HitTable.GetSizeInBytes();
		desc.HitGroupTable.StrideInBytes	= RaytracingShaderTable->HitTable.GetStride();
		
		/*
		desc.RayGenerationShaderRecord.StartAddress = (UINT64)RaytracingShaderTable->RayGenTable.MappedData + RaytracingShaderTable->RayGenOffset;
		desc.RayGenerationShaderRecord.SizeInBytes = RaytracingShaderTable->RayGenTable.GetSizeInBytes();

		desc.MissShaderTable.StartAddress = (UINT64)RaytracingShaderTable->MissTable.MappedData;
		desc.MissShaderTable.SizeInBytes = RaytracingShaderTable->MissTable.GetSizeInBytes();
		desc.MissShaderTable.StrideInBytes = RaytracingShaderTable->MissTable.GetStride();

		desc.HitGroupTable.StartAddress = (UINT64)RaytracingShaderTable->HitTable.MappedData;
		desc.HitGroupTable.SizeInBytes = RaytracingShaderTable->HitTable.GetSizeInBytes();
		desc.HitGroupTable.StrideInBytes = RaytracingShaderTable->HitTable.GetStride();
		*/
		desc.Depth = 1;

		CurrentFrame.GraphicsCommandList->GetHandle()->DispatchRays(&desc);
		
	}

} // namespace Luden

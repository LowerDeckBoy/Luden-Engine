#include "Renderer.hpp"
#include "D3D12/D3D12Utility.hpp"
#include <Core/Logger.hpp>

namespace Luden
{
	SceneRenderTargets Renderer::SceneTextures = {};

	Renderer::Renderer(Platform::Window* pParentWindow, D3D12RHI* pD3D12RHI)
		: m_ParentWindow(pParentWindow),
		m_D3D12RHI(pD3D12RHI)
	{
		SceneTextures.Scene.Create(m_D3D12RHI->Device, {
			.Usage  = TextureUsageFlag::RenderTarget,
			.pData  = nullptr,
			.Width  = static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Width),
			.Height = static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Height),
			.Format = m_D3D12RHI->SwapChain->GetSwapChainFormat(),
			},
			"Scene Output Render Texture");

		m_ShaderCompiler = new ShaderCompiler();

		Camera = new SceneCamera(pParentWindow);

		BaseVS = m_ShaderCompiler->CompileVS("../../Shaders/Base.hlsl");
		BasePS = m_ShaderCompiler->CompilePS("../../Shaders/Base.hlsl");

		VertexVS = m_ShaderCompiler->CompileVS("../../Shaders/Test.hlsl");
		VertexPS = m_ShaderCompiler->CompilePS("../../Shaders/Test.hlsl");

		MeshMS = m_ShaderCompiler->CompileMS("../../Shaders/MS.hlsl");
		MeshPS = m_ShaderCompiler->CompilePS("../../Shaders/MS.hlsl");


		BuildPipelines();

		//_D3D12RHI->Wait();
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
		

		//frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// TEST

		
		frame->GraphicsCommandList->ClearDepthStencilView(depthStencilView);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetCpuStartHandle(), BackBufferIndex, m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetDescriptorIncrementSize());
		//frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthStencilView.CpuHandle);
		frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_RENDER_TARGET);
		constexpr float clearColor[4] = { 0.5f, 0.2f, 0.7f, 1.0f };
		frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(SceneTextures.Scene.RenderTargetHandle.CpuHandle, clearColor, 0, nullptr);
		frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &SceneTextures.Scene.RenderTargetHandle.CpuHandle, FALSE, &depthStencilView.CpuHandle);

		// TEST

		//frame->GraphicsCommandList->SetRootSignature(&BaseRS);
		//frame->GraphicsCommandList->SetPipelineState(&BasePSO);

		frame->GraphicsCommandList->GetHandle()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//frame->GraphicsCommandList->SetRootSignature(&BaseRS);
		//frame->GraphicsCommandList->SetPipelineState(&BasePSO);
		//frame->GraphicsCommandList->GetHandle()->DrawInstanced(3, 1, 0, 0);

		//frame->GraphicsCommandList->SetRootSignature(&VertexRS);
		//frame->GraphicsCommandList->SetPipelineState(&VertexPSO);

		Draw(pScene, *frame);

		//frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
		//frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_COPY_DEST);
		//frame->GraphicsCommandList->CopyResource(&backbuffer, &SceneTextures.Scene);
		//frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_GENERIC_READ);
		frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_GENERIC_READ);

		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthStencilView.CpuHandle);
		frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	//	frame->GraphicsCommandList->ClearDepthStencilView(depthStencilView);

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
		if (pScene->Models.empty())
		{
			return;
		}

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
				model.cbPerObject = { 
					DirectX::XMMatrixTranspose(transform.WorldMatrix * Camera->GetViewProjection()), 
					DirectX::XMMatrixTranspose(transform.WorldMatrix) 
					};
				constantBuffer->Update(&model.cbPerObject);
				CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);

				for (auto& mesh : model.Meshes)
				{
					uint32 vertexBuffer				= device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
					uint32 meshletBuffer			= device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
					uint64 meshletVerticesBuffer	= device->Buffers.at(mesh.MeshletsVerticesBuffer)->GetGpuAddress();
					uint64 meshletTrianglesBuffer	= device->Buffers.at(mesh.MeshletsTrianglesBuffer)->GetGpuAddress();

					struct
					{
						uint32 vertex;
						uint32 meshlet;
						uint32 bDrawMeshlets;
					} buffers
					{
						.vertex = vertexBuffer,
						.meshlet = meshletBuffer,
						.bDrawMeshlets = (uint32)Config::Get().bDrawMeshlets
					};

					CurrentFrame.GraphicsCommandList->PushConstants(1, 3, &buffers);
					CurrentFrame.GraphicsCommandList->PushRootSRV(2, meshletVerticesBuffer);
					CurrentFrame.GraphicsCommandList->PushRootSRV(3, meshletTrianglesBuffer);

					CurrentFrame.GraphicsCommandList->DispatchMesh(mesh.NumMeshlets, 1, 1);

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

				model.cbPerObject = { 
					DirectX::XMMatrixTranspose(transform.WorldMatrix * Camera->GetViewProjection()), 
					DirectX::XMMatrixTranspose(transform.WorldMatrix) 
					};
				constantBuffer->Update(&model.cbPerObject);

				for (auto& mesh : model.Meshes)
				{
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
		// Triangle
		{
			//BaseRS.AddConstants(1, 0);
			//BaseRS.AddStaticSampler(0, 0, D3D12_FILTER_MAXIMUM_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
			//VERIFY_D3D12_RESULT(BaseRS.Build(m_D3D12RHI->Device, PipelineType::Graphics));
			//
			//D3D12PipelineStateBuilder builder(m_D3D12RHI->Device);
			//builder.SetRootSignature(&BaseRS);
			//builder.SetVertexShader(&BaseVS);
			//builder.SetPixelShader(&BasePS);
			//builder.SetRenderTargetFormats({ m_D3D12RHI->SwapChain->GetSwapChainFormat() });
			////builder.
			//
			//VERIFY_D3D12_RESULT(builder.Build(BasePSO));
		}
	
		// Vertex
		{
			VertexRS.AddCBV(0);				// Transform matrices
			VertexRS.AddConstants(1, 1);	// Index to Vertex Buffer
			VertexRS.AddStaticSampler(0, 0, D3D12_FILTER_MAXIMUM_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
			VERIFY_D3D12_RESULT(VertexRS.Build(m_D3D12RHI->Device, PipelineType::Graphics));

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
			MeshRS.AddCBV(0);			// Transform matrices
			MeshRS.AddConstants(3, 1);	// Vertex and Meshlet Buffers
			MeshRS.AddSRV(0);			// Meshlet Vertices Buffer
			MeshRS.AddSRV(1);			// Meshlet Triangles Buffer
			MeshRS.AddStaticSampler(0, 0, D3D12_FILTER_MAXIMUM_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
			VERIFY_D3D12_RESULT(MeshRS.Build(m_D3D12RHI->Device, PipelineType::Graphics));

			D3D12MeshPipelineStateBuilder builder(m_D3D12RHI->Device);
			builder.SetRootSignature(&MeshRS);
			builder.SetMeshShader(&MeshMS);
			builder.SetPixelShader(&MeshPS);
			builder.EnableDepth(true);
			builder.SetCullMode(D3D12_CULL_MODE_BACK);
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

		D3D12UploadContext::Upload();

	}

} // namespace Luden

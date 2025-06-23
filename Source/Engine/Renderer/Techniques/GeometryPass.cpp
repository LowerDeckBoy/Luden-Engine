#include "Scene/Scene.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "GeometryPass.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	GeometryPass::GeometryPass(D3D12RHI* pRHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pRHI)
	{
		Initialize(pRHI, Width, Height);
		CreatePipeline(pShaderCompiler);
	}

	GeometryPass::~GeometryPass()
	{
		Release();
	}

	void GeometryPass::Initialize(D3D12RHI* pRHI, uint32 Width, uint32 Height)
	{
		Release();

		DXGI_FORMAT swapChainFormat = pRHI->SwapChain->GetSwapChainFormat();

		//RenderTargetClearColor
		BaseColor.Create(pRHI->Device, Width, Height, swapChainFormat, RenderTargetClearColor, "GBuffer BaseColor");
		Normal.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R32G32B32A32_FLOAT, RenderTargetClearColor, "GBuffer Normal");
		MetallicRoughness.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, RenderTargetClearColor, "GBuffer MetallicRoughness");
		Emissive.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, RenderTargetClearColor, "GBuffer Emissive");

		m_RenderTargetHandles.push_back(&BaseColor.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Normal.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&MetallicRoughness.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Emissive.RenderTargetHandle);

	}

	void GeometryPass::CreatePipeline(ShaderCompiler* pShaderCompiler)
	{
		// Opaque PSO
		{
			Pipeline.Amplification = pShaderCompiler->CompileAS("../../Shaders/Mesh/GBuffer_AS.hlsl", false);
			Pipeline.Mesh = pShaderCompiler->CompileMS("../../Shaders/Mesh/GBuffer_MS.hlsl", true);
			Pipeline.Pixel = pShaderCompiler->CompilePS("../../Shaders/Mesh/GBuffer_PS.hlsl", false);

			VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(m_RHI->Device, &Pipeline.Mesh, PipelineType::Graphics));

			D3D12MeshPipelineStateBuilder builder(m_RHI->Device);
			builder.SetRootSignature(&Pipeline.RootSignature);
			builder.SetAmplificationShader(&Pipeline.Amplification);
			builder.SetMeshShader(&Pipeline.Mesh);
			builder.SetPixelShader(&Pipeline.Pixel);
			builder.EnableDepth(true);
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
			builder.SetAlphaModeOpaque(0);
			builder.SetAlphaModeOpaque(1);
			builder.SetAlphaModeOpaque(2);
			builder.SetAlphaModeOpaque(3);
			builder.SetRenderTargetFormats({
				BaseColor.GetFormat(),
				Normal.GetFormat(),
				MetallicRoughness.GetFormat(),
				Emissive.GetFormat()
				});

			VERIFY_D3D12_RESULT(builder.Build(Pipeline.PipelineState));
		}

		// Blend PSO
		{
			BlendPipelineState.Amplification = pShaderCompiler->CompileAS("../../Shaders/Mesh/GBufferBlend_MS.hlsl", false);
			BlendPipelineState.Mesh = pShaderCompiler->CompileMS("../../Shaders/Mesh/GBufferBlend_MS.hlsl", true);
			BlendPipelineState.Pixel = pShaderCompiler->CompilePS("../../Shaders/Mesh/GBufferBlend_MS.hlsl", false);

			VERIFY_D3D12_RESULT(BlendPipelineState.RootSignature.BuildFromShader(m_RHI->Device, &BlendPipelineState.Mesh, PipelineType::Graphics));

			D3D12MeshPipelineStateBuilder builder(m_RHI->Device);
			builder.SetRootSignature(&BlendPipelineState.RootSignature);
			builder.SetAmplificationShader(&BlendPipelineState.Amplification);
			builder.SetMeshShader(&BlendPipelineState.Mesh);
			builder.SetPixelShader(&BlendPipelineState.Pixel);
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
			builder.SetAlphaModeBlend(0);
			//builder.SetAlphaModeAdditive(0);
			builder.SetAlphaBlendDepthDesc();
			builder.SetRenderTargetFormats({
				BaseColor.GetFormat(),
				//Normal.GetFormat(),
				//MetallicRoughness.GetFormat(),
				//Emissive.GetFormat()
				});

			VERIFY_D3D12_RESULT(builder.Build(BlendPipelineState.PipelineState));
		}
		
	}

	void GeometryPass::Release()
	{
		BaseColor.Release();
		Normal.Release();
		MetallicRoughness.Release();
		Emissive.Release();

		m_RenderTargetHandles.clear();
		m_RenderTargetHandles.shrink_to_fit();

	}

	void GeometryPass::Resize(uint32 Width, uint32 Height)
	{
		BaseColor.Resize(Width, Height);
		Normal.Resize(Width, Height);
		MetallicRoughness.Resize(Width, Height);
		Emissive.Resize(Width, Height);
	}
	
	void GeometryPass::Render(Frame& CurrentFrame, std::function<void()> const& DrawFunction)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&Pipeline.PipelineState);

		commandList->ResourceTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Normal,				D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Emissive,			D3D12_RESOURCE_STATE_RENDER_TARGET } });

		commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTargets(m_RenderTargetHandles, RenderTargetClearColor);

		DrawFunction();

		commandList->ResourceTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Normal,				D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Emissive,			D3D12_RESOURCE_STATE_GENERIC_READ } });
	}

	void GeometryPass::Render(Scene* pScene, SceneCamera* pCamera, Frame& CurrentFrame)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;

		//auto& config = Config::Get();

		commandList->ResourceTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Normal,				D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Emissive,			D3D12_RESOURCE_STATE_RENDER_TARGET } });

		commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTargets(m_RenderTargetHandles, RenderTargetClearColor);

		auto device = m_RHI->Device;

		commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&Pipeline.PipelineState);

		commandList->PushConstants(3, 28, &pScene->Consts);

		// Draw all opaque objects.
		for (auto& model : pScene->Models)
		{
			if (model->OpaqueMeshes.empty()) continue;

			auto& transform = model->GetComponent<ecs::TransformComponent>();
			transform.Update();

			auto* constantBuffer = device->ConstantBuffers.at(model->ConstantBuffer);

			model->cbObjectTransforms.WVP = DirectX::XMMatrixTranspose(transform.WorldMatrix * pCamera->GetViewProjection());
			model->cbObjectTransforms.World = DirectX::XMMatrixTranspose(transform.WorldMatrix);
			constantBuffer->Update(&model->cbObjectTransforms);

			CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);
			
			for (auto& mesh : model->OpaqueMeshes)
			{
				uint32 vertexBuffer = device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
				uint32 meshletBuffer = device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
				uint32 meshletVerticesBuffer = device->Buffers.at(mesh.MeshletVerticesBuffer)->ShaderResourceView.Index;
				uint32 meshletTrianglesBuffer = device->Buffers.at(mesh.MeshletTrianglesBuffer)->ShaderResourceView.Index;
				uint32 meshletBoundsBuffer = device->Buffers.at(mesh.MeshletBoundsBuffer)->ShaderResourceView.Index;

				struct
				{
					uint32 vertex;
					uint32 meshlet;
					uint32 meshletVertices;
					uint32 meshletTriangles;
					uint32 meshletBounds;
					uint32 bDrawMeshlets;
					uint32 bMeshletCulling;
					uint32 bAlphaMask;
					uint32 materialBuffer;
					uint32 materialID;
				} buffers
				{
					.vertex				= vertexBuffer,
					.meshlet			= meshletBuffer,
					.meshletVertices	= meshletVerticesBuffer,
					.meshletTriangles	= meshletTrianglesBuffer,
					.meshletBounds		= meshletBoundsBuffer,
					.bDrawMeshlets		= (uint32)Config::Get().bDrawMeshlets,
					.bMeshletCulling	= (uint32)Config::Get().bMeshletCulling,
					.bAlphaMask			= (uint32)Config::Get().bAlphaMask,
					.materialBuffer		= pScene->MaterialBuffer->ShaderResourceView.Index,
					.materialID			= mesh.MaterialId,
				};
			
				CurrentFrame.GraphicsCommandList->PushConstants(1, 10 , &buffers);
				//CurrentFrame.GraphicsCommandList->PushConstants(2, 20, &material);

				CurrentFrame.GraphicsCommandList->DispatchMesh(Math::RoundUp<uint32>(mesh.NumMeshlets / 32), 1, 1);
			}	
		}

		commandList->SetRootSignature(&BlendPipelineState.RootSignature);
		commandList->SetPipelineState(&BlendPipelineState.PipelineState);

		// Draw all blend/transparent objects.
		for (auto& model : pScene->Models)
		{
			if (model->BlendMeshes.empty()) continue;
			
			auto& transform = model->GetComponent<ecs::TransformComponent>();
			transform.Update();

			auto* constantBuffer = device->ConstantBuffers.at(model->ConstantBuffer);

			model->cbObjectTransforms.WVP = DirectX::XMMatrixTranspose(transform.WorldMatrix * pCamera->GetViewProjection());
			model->cbObjectTransforms.World = DirectX::XMMatrixTranspose(transform.WorldMatrix);
			constantBuffer->Update(&model->cbObjectTransforms);

			CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);

			for (auto it = model->BlendMeshes.begin(); it != model->BlendMeshes.end(); ++it)
			{
				auto& mesh = *it;

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
					uint32 bMeshletCulling;
					//uint32 bAlphaMask;
					uint32 gbufferBaseColor;
				} buffers
				{
					.vertex = vertexBuffer,
					.meshlet = meshletBuffer,
					.meshletVertices = meshletVerticesBuffer,
					.meshletTriangles = meshletTrianglesBuffer,
					.meshletBounds = meshletBoundsBuffer,
					.bDrawMeshlets	 = (uint32)Config::Get().bDrawMeshlets,
					.bMeshletCulling = (uint32)Config::Get().bMeshletCulling,
					//.bAlphaMask = (uint32)Config::Get().bAlphaMask,
					.gbufferBaseColor = BaseColor.ShaderResourceHandle.Index
				};

				CurrentFrame.GraphicsCommandList->PushConstants(1, 8, &buffers);
				CurrentFrame.GraphicsCommandList->PushConstants(2, 20, &material);

				CurrentFrame.GraphicsCommandList->DispatchMesh(ROUND_UP(mesh.NumMeshlets / 32), 1, 1);
			}
		}

		commandList->ResourceTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Normal,				D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Emissive,			D3D12_RESOURCE_STATE_GENERIC_READ } });
	}

} // namespace Luden

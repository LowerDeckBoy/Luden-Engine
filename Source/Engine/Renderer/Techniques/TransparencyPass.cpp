#include "D3D12/D3D12Device.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "Scene/Scene.hpp"
#include "TransparencyPass.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	TransparencyPass::TransparencyPass(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler)
		: RenderPass(pD3D12RHI)
	{
		Pipeline.Amplification	= pShaderCompiler->CompileAS("../../Shaders/Mesh/GBuffer_AS.hlsl", false);
		Pipeline.Mesh			= pShaderCompiler->CompileMS("../../Shaders/Mesh/GBuffer_MS.hlsl", true);
		Pipeline.Pixel			= pShaderCompiler->CompilePS("../../Shaders/Forward/Transparency_PS.hlsl", false);

		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(m_RHI->Device, &Pipeline.Mesh, PipelineType::Graphics));

		D3D12MeshPipelineStateBuilder builder(m_RHI->Device);
		builder.SetRootSignature(&Pipeline.RootSignature);
		builder.SetAmplificationShader(&Pipeline.Amplification);
		builder.SetMeshShader(&Pipeline.Mesh);
		builder.SetPixelShader(&Pipeline.Pixel);
		builder.SetCullMode(D3D12_CULL_MODE_NONE);
		builder.SetAlphaModeBlend(0);
		builder.SetAlphaBlendDepthDesc();
		builder.SetRenderTargetFormats({ DXGI_FORMAT_R11G11B10_FLOAT });
		//builder.SetRenderTargetFormats({ DXGI_FORMAT_R32G32B32A32_FLOAT });

		VERIFY_D3D12_RESULT(builder.Build(Pipeline.PipelineState));
	}

	TransparencyPass::~TransparencyPass()
	{
	}

	void TransparencyPass::Render(Frame& CurrentFrame, D3D12RenderTexture* pRenderTarget, D3D12Descriptor* pDepthStencil, Scene* pScene)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;
		auto device = m_RHI->Device;
		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetRootSignature(&Pipeline.RootSignature);

		commandList->ResourceTransition(pRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetRenderTargets(pRenderTarget->RenderTargetHandle, *pDepthStencil);

		commandList->PushConstants(1, 28, &pScene->Consts);
		commandList->SetConstantBuffer(2, pScene->SceneDataBuffer);

		for (auto& model : pScene->Models)
		{
			if (model->BlendMeshes.empty())
			{
				continue;
			}

			for (auto& mesh : model->BlendMeshes)
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
					uint32 DepthIndex;
					uint32 transformsBuffer;
					uint32 materialBuffer;
					uint32 materialID;
					uint32 transformID;
					uint32 pointLightsBufferIndex;
					uint32 farZ = 1;
				} buffers
				{
					.vertex = vertexBuffer,
					.meshlet = meshletBuffer,
					.meshletVertices = meshletVerticesBuffer,
					.meshletTriangles = meshletTrianglesBuffer,
					.meshletBounds = meshletBoundsBuffer,
					.bDrawMeshlets = (uint32)Config::Get().bDrawMeshlets,
					.bMeshletCulling = (uint32)Config::Get().bMeshletCulling,
					.DepthIndex = m_RHI->SceneDepthBuffer->ShaderResourceHandle.Index,
					.transformsBuffer = pScene->TransformsBuffer->ShaderResourceView.Index,
					.materialBuffer = pScene->MaterialBuffer->ShaderResourceView.Index,
					.materialID = mesh.MaterialID,
					.transformID = model->TransformID,
					.pointLightsBufferIndex = pScene->LightBuffer->ShaderResourceView.Index,
				};

				commandList->PushConstants(0, 14, &buffers);

				commandList->DispatchMesh(Math::RoundUp<uint32>(mesh.NumMeshlets / 32), 1, 1);
			}
		}	

		commandList->ResourceTransition(pRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);

	}

	void TransparencyPass::Release()
	{
	}

} // namespace Luden

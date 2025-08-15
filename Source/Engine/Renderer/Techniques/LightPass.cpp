#include "Asset/ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"
#include "LightPass.hpp"
#include "ECS/Components/LightComponent.hpp"


namespace Luden
{
	LightPass::LightPass(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, GeometryPass* pGeometryPass, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{
		Initialize(pD3D12RHI, pShaderCompiler, pGeometryPass, Width, Height);
	}

	LightPass::~LightPass()
	{
		Release();
	}

	void LightPass::Initialize(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, GeometryPass* pGeometryPass, uint32 Width, uint32 Height)
	{
		m_RHI = pD3D12RHI;
		m_GeometryPass = pGeometryPass;

		RenderTexture.Create(m_RHI->Device, Width, Height, pD3D12RHI->SwapChain->GetSwapChainFormat(), RenderTargetClearColor, "Light Pass Render Target");
		//RenderTexture.Create(m_RHI->Device, Width, Height, DXGI_FORMAT_R32G32B32A32_FLOAT, DefaultClearColor, "Light Pass Render Target");

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Format = RenderTexture.GetFormat();
		m_RHI->Device->ShaderResourceHeap->Allocate(RenderTexture.UnorderedAccessHandle, 1);
		m_RHI->Device->LogicalDevice->CreateUnorderedAccessView(RenderTexture.GetHandleRaw(), nullptr, &uavDesc, RenderTexture.UnorderedAccessHandle.CpuHandle);

		Pipeline.Vertex = pShaderCompiler->CompileVS("../../Shaders/Deferred/Deferred.hlsl", false);
		Pipeline.Pixel  = pShaderCompiler->CompilePS("../../Shaders/Deferred/Deferred.hlsl", true);

		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(pD3D12RHI->Device, &Pipeline.Pixel, PipelineType::Graphics));

		D3D12PipelineStateBuilder builder(pD3D12RHI->Device);
		builder.EnableDepth(false);
		builder.SetCullMode(D3D12_CULL_MODE_NONE);
		builder.SetVertexShader(&Pipeline.Vertex);
		builder.SetPixelShader(&Pipeline.Pixel);
		builder.SetRenderTargetFormats({ RenderTexture.GetFormat() });

		VERIFY_D3D12_RESULT(builder.Build(Pipeline.PipelineState));

		// Test
		// Compute PSO
		{
			ComputePSO.Compute = pShaderCompiler->CompileCS("../../Shaders/Deferred/Deferred_CS.hlsl", true);
			D3D12ComputePipelineStateBuilder csBuilder;
			csBuilder.SetComputeShader(&ComputePSO.Compute);
			VERIFY_D3D12_RESULT(ComputePSO.RootSignature.BuildFromShader(pD3D12RHI->Device, &ComputePSO.Compute, PipelineType::Compute));
			csBuilder.SetRootSignature(&ComputePSO.RootSignature);
			VERIFY_D3D12_RESULT(csBuilder.Build(m_RHI->Device, ComputePSO));
		}
		

	}

	void LightPass::Render(Scene* pScene, Frame& CurrentFrame, SceneCamera* pCamera)
	{
		auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&Pipeline.PipelineState);

		commandList->ResourceTransition(&RenderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

		//commandList->SetRenderTargets(RenderTexture.RenderTargetHandle, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		//commandList->ClearRenderTarget(RenderTexture.RenderTargetHandle, DefaultClearColor);
		commandList->ClearRenderTarget(RenderTexture.RenderTargetHandle, RenderTargetClearColor);
		commandList->SetRenderTargets(RenderTexture.RenderTargetHandle);

		pScene->UpdateSceneBufferData(pCamera);

		// Push Constants here
		struct pushConstants
		{
			uint32 PointLightBufferIndex;
			uint32 NumPointLights;
			uint32 SpotLightBufferIndex;
			uint32 NumSpotLights;
			uint32 BaseColorIndex;
			uint32 NormalIndex;
			uint32 MRIndex;
			uint32 EmissiveIndex;
			uint32 WorldPositionIndex;
			uint32 padding = 0;
		} constants{
			.PointLightBufferIndex	= pScene->LightBuffer->ShaderResourceView.Index,
			.NumPointLights			= static_cast<uint32>(pScene->PointLights.size()),
			.SpotLightBufferIndex	= pScene->SpotLightBuffer->ShaderResourceView.Index,
			.NumSpotLights			= static_cast<uint32>(pScene->SpotLights.size()),
			.BaseColorIndex			= m_GeometryPass->BaseColor.ShaderResourceHandle.Index,
			.NormalIndex			= m_GeometryPass->Normal.ShaderResourceHandle.Index,
			.MRIndex				= m_GeometryPass->MetallicRoughness.ShaderResourceHandle.Index,
			.EmissiveIndex			= m_GeometryPass->Emissive.ShaderResourceHandle.Index,
			.WorldPositionIndex		= m_GeometryPass->WorldPosition.ShaderResourceHandle.Index,
		};

		commandList->PushConstants(2, 10, &constants);
		
		commandList->SetConstantBuffer(1, pScene->SceneDataBuffer);

		// Draw screen space quad.
		commandList->Draw(4);

		commandList->ResourceTransition(&RenderTexture, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime).count();
	}

	void LightPass::RenderCompute(Scene* pScene, Frame& CurrentFrame, SceneCamera* pCamera)
	{
		auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->SetPipelineState(&ComputePSO.PipelineState);
		commandList->SetRootSignature(&ComputePSO.RootSignature);

		commandList->ResourceTransition(&RenderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		pScene->UpdateSceneBufferData(pCamera);

		// Push Constants here
		struct pushConstants
		{
			uint32 PointLightBufferIndex;
			uint32 NumPointLights;
			uint32 SpotLightBufferIndex;
			uint32 NumSpotLights;
			uint32 BaseColorIndex;
			uint32 NormalIndex;
			uint32 MRIndex;
			uint32 EmissiveIndex;
			uint32 WorldPositionIndex;
			uint32 OutputImage;
		} constants{
			.PointLightBufferIndex	= pScene->LightBuffer->ShaderResourceView.Index,
			.NumPointLights			= static_cast<uint32>(pScene->PointLights.size()),
			.SpotLightBufferIndex	= pScene->SpotLightBuffer->ShaderResourceView.Index,
			.NumSpotLights			= static_cast<uint32>(pScene->SpotLights.size()),
			.BaseColorIndex			= m_GeometryPass->BaseColor.ShaderResourceHandle.Index,
			.NormalIndex			= m_GeometryPass->Normal.ShaderResourceHandle.Index,
			.MRIndex				= m_GeometryPass->MetallicRoughness.ShaderResourceHandle.Index,
			.EmissiveIndex			= m_GeometryPass->Emissive.ShaderResourceHandle.Index,
			.WorldPositionIndex		= m_GeometryPass->WorldPosition.ShaderResourceHandle.Index,
			.OutputImage			= RenderTexture.ShaderResourceHandle.Index,
		};

		commandList->GetHandle()->SetComputeRoot32BitConstants(2, 10, &constants, 0);
		commandList->GetHandle()->SetComputeRootConstantBufferView(1, pScene->SceneDataBuffer->GetBuffer()->GetGPUVirtualAddress());
		//commandList->SetConstantBuffer(1, pScene->SceneDataBuffer);

		// Draw screen space quad.
		//commandList->Draw(4);
		commandList->Dispatch(Math::RoundUp<uint32>((uint32)RenderTexture.GetDesc().Width / 8), Math::RoundUp<uint32>(RenderTexture.GetDesc().Height / 8), 1);

		commandList->ResourceTransition(&RenderTexture, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime).count();
	}

	void LightPass::Resize(uint32 Width, uint32 Height)
	{
		RenderTexture.Resize(Width, Height);
	}

	void LightPass::Release()
	{
		RenderTexture.Release();
	}

} // namespace Luden

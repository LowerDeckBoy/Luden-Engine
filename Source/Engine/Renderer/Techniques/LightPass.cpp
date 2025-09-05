#include "Asset/ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"
#include "ECS/Components/LightComponent.hpp"
#include "LightPass.hpp"


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

		Pipeline.Compute = pShaderCompiler->CompileCS("../../Shaders/Deferred/Deferred.hlsl", true);
		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(pD3D12RHI->Device, &Pipeline.Compute, PipelineType::Compute));

		D3D12ComputePipelineStateBuilder csBuilder;
		csBuilder.SetComputeShader(&Pipeline.Compute);
		csBuilder.SetRootSignature(&Pipeline.RootSignature);
		VERIFY_D3D12_RESULT(csBuilder.Build(m_RHI->Device, Pipeline));
	}

	void LightPass::Render(Scene* pScene, Frame& CurrentFrame, SceneCamera* pCamera)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.ComputeCommandList;

		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetRootSignature(&Pipeline.RootSignature);

		commandList->ResourceTransition(&RenderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		pScene->UpdateSceneBufferData(pCamera);

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

		commandList->SetComputeConstantBuffer(1, pScene->SceneDataBuffer);
		commandList->PushComputeConstants(2, 10, &constants);

		commandList->Dispatch(Math::RoundUp<uint32>((uint32)RenderTexture.GetDesc().Width / 8), Math::RoundUp<uint32>(RenderTexture.GetDesc().Height / 8), 1);

		commandList->ResourceTransition(&RenderTexture, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
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

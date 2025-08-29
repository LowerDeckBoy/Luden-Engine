#include "SSR.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"
#include "Scene/SceneCamera.hpp"

namespace Luden
{
	SSR::SSR(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{
		Pipeline.Compute = pShaderCompiler->CompileCS("../../Shaders/SSR/SSR.hlsl", true);
		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(pD3D12RHI->Device, &Pipeline.Compute, PipelineType::Compute));

		D3D12ComputePipelineStateBuilder builder;
		builder.SetComputeShader(&Pipeline.Compute);
		builder.SetRootSignature(&Pipeline.RootSignature);
		VERIFY_D3D12_RESULT(builder.Build(pD3D12RHI->Device, Pipeline));

		RenderTarget.Create(pD3D12RHI->Device, Width, Height, DXGI_FORMAT_R32G32B32A32_FLOAT, RenderTargetClearColor, "Screen Space Reflections Render Target");

	}

	SSR::~SSR()
	{
	}

	void SSR::Render(Frame* CurrentFrame, uint32 SceneImageIndex, uint32 NormalsIndex, uint32 RoughnessIndex, uint32 WorldPositionIndex, uint32 DepthIndex, SceneCamera* pCamera)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame->ComputeCommandList;

		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetComputeRootSignature(&Pipeline.RootSignature);

		commandList->ResourceTransition(&RenderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		//Parameters.View					= (pCamera->GetView());
		Parameters.Projection			= (pCamera->GetProjection());
		Parameters.InversedView			= DirectX::XMMatrixTranspose(pCamera->GetInversedView());
		Parameters.InversedProjection   = DirectX::XMMatrixTranspose(pCamera->GetInversedProjection());
		Parameters.NormalIndex			= NormalsIndex;
		Parameters.RoughnessIndex		= RoughnessIndex;
		Parameters.WorldPositionIndex	= WorldPositionIndex;
		Parameters.DepthIndex			= DepthIndex;
		Parameters.InputImageIndex		= SceneImageIndex;
		Parameters.OutputImageIndex		= RenderTarget.ShaderResourceHandle.Index;

		commandList->PushComputeConstants(0, 54, &Parameters);

		const uint32 dispatchX = Math::RoundUp((uint32)RenderTarget.GetDesc().Width  / 8u);
		const uint32 dispatchY = Math::RoundUp((uint32)RenderTarget.GetDesc().Height / 8u);
		commandList->Dispatch(dispatchX, dispatchY, 1);

		commandList->ResourceTransition(&RenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
			
		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
	}

	void SSR::Resize(uint32 Width, uint32 Height)
	{
		RenderTarget.Resize(Width, Height);
	}

	void SSR::Release()
	{
	}
} // namespace Luden

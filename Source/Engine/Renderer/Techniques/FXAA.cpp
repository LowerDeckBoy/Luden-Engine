#include "FXAA.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	FXAA::FXAA(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{
		// Do I need it for other reason than debug?
		RenderTarget.Create(pD3D12RHI->Device, Width, Height, pD3D12RHI->SwapChain->GetSwapChainFormat(), RenderTargetClearColor, "FXAA Render Target");

		CreatePipelines(pShaderCompiler);
	}

	FXAA::~FXAA()
	{
		Release();
	}

	void FXAA::Render(Frame& CurrentFrame, uint32 SceneImageIndex)
	{
		auto commandList = CurrentFrame.ComputeCommandList;
		const uint32 dispatchBlock = 8;

		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetComputeRootSignature(&Pipeline.RootSignature);

		Parameters.SceneImageIndex = SceneImageIndex;
		Parameters.DebugImageIndex = RenderTarget.ShaderResourceHandle.Index;

		commandList->GetHandle()->SetComputeRoot32BitConstants(0, 5, &Parameters, 0);

		const uint32 dispatchX = Math::RoundUp<uint32>((uint32)RenderTarget.GetDesc().Width / dispatchBlock);
		const uint32 dispatchY = Math::RoundUp<uint32>(RenderTarget.GetDesc().Height / dispatchBlock);
		commandList->Dispatch(dispatchX, dispatchY, 1);

	}

	void FXAA::Resize(uint32 Width, uint32 Height)
	{
		RenderTarget.Resize(Width, Height);
	}

	void FXAA::Release()
	{
	}

	void FXAA::CreatePipelines(ShaderCompiler* pShaderCompiler)
	{
		Pipeline.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/FXAA/FXAA.hlsl", true);

		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(m_RHI->Device, &Pipeline.Compute, PipelineType::Compute));

		D3D12ComputePipelineStateBuilder builder;
		builder.SetComputeShader(&Pipeline.Compute);
		builder.SetRootSignature(&Pipeline.RootSignature);
		VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, Pipeline));
	}

} // namespace Luden

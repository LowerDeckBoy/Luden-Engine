#include "Asset/ShaderCompiler.hpp"
#include "Scattering.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	Scattering::Scattering(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{

		RenderTarget.Create(pD3D12RHI->Device, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT);

		Pipeline.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/Volumetric/Scattering.hlsl", true);

		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(pD3D12RHI->Device, &Pipeline.Compute, PipelineType::Compute));

		D3D12ComputePipelineStateBuilder builder;
		builder.SetComputeShader(&Pipeline.Compute);
		builder.SetRootSignature(&Pipeline.RootSignature);
		VERIFY_D3D12_RESULT(builder.Build(pD3D12RHI->Device, Pipeline));
	}

	Scattering::~Scattering()
	{
	}

	void Scattering::Render(Frame& CurrentFrame, uint32 SceneImageIndex, DirectX::XMFLOAT3 SunPosition)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.ComputeCommandList;
		
		commandList->ResourceTransition(&RenderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetComputeRootSignature(&Pipeline.RootSignature);

		Parameters.SceneImageIndex		= SceneImageIndex;
		Parameters.OutputImageIndex		= RenderTarget.ShaderResourceHandle.Index;
		Parameters.ScreenLightPosition	= SunPosition;

		commandList->PushComputeConstants(0, 4, &Parameters);

		const uint32 dispatchBlock = 8;
		const uint32 dispatchX = Math::RoundUp((uint32)RenderTarget.GetDesc().Width  / dispatchBlock);
		const uint32 dispatchY = Math::RoundUp((uint32)RenderTarget.GetDesc().Height / dispatchBlock);
		commandList->Dispatch(dispatchX, dispatchY, 1);

		commandList->ResourceTransition(&RenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);

	}

	void Scattering::Resize(uint32 Width, uint32 Height)
	{
		RenderTarget.Resize(Width, Height);
	}

	void Scattering::Release()
	{
	}
} // namespace Luden

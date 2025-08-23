#include "Tonemapping.hpp"
#include "D3D12/D3D12Utility.hpp"
#include "Asset/ShaderCompiler.hpp"
#include <Core/Time/Time.hpp>

namespace Luden
{
	Tonemapping::Tonemapping(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler)
	{

		PSO.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/Tonemapping/Tonemapping.hlsl", true);

		VERIFY_D3D12_RESULT(PSO.RootSignature.BuildFromShader(pD3D12RHI->Device, &PSO.Compute, PipelineType::Compute));

		D3D12ComputePipelineStateBuilder builder;
		builder.SetComputeShader(&PSO.Compute);
		builder.SetRootSignature(&PSO.RootSignature);
		VERIFY_D3D12_RESULT(builder.Build(pD3D12RHI->Device, PSO));
		
	}

	Tonemapping::~Tonemapping()
	{

	}

	void Tonemapping::Render(Frame& CurrentFrame, uint32 SceneImageIndex, uint32 Width, uint32 Height)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.ComputeCommandList;

		commandList->SetPipelineState(&PSO.PipelineState);
		commandList->SetRootSignature(&PSO.RootSignature);

		struct params
		{
			uint32		ImageIndex;
			float		Exposure;
			int32		Type;
		} constants
		{
			.ImageIndex = SceneImageIndex,
			.Exposure	= Exposure,
			.Type		= Type
		};

		commandList->GetHandle()->SetComputeRoot32BitConstants(0, 3, &constants, 0);

		const uint32 dispatchX = Math::RoundUp(Width  / 8u);
		const uint32 dispatchY = Math::RoundUp(Height / 8u);
		commandList->Dispatch(dispatchX, dispatchY, 1);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
	}

} // namespace Luden

#include "FilmEffects.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	FilmEffects::FilmEffects(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{
		Pipeline.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/FilmEffects/FilmEffects.hlsl", true);
		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(pD3D12RHI->Device, &Pipeline.Compute, PipelineType::Compute));

		D3D12ComputePipelineStateBuilder builder;
		builder.SetComputeShader(&Pipeline.Compute);
		builder.SetRootSignature(&Pipeline.RootSignature);
		VERIFY_D3D12_RESULT(builder.Build(pD3D12RHI->Device, Pipeline));

		RenderTarget.Create(pD3D12RHI->Device, Width, Height, DXGI_FORMAT_R10G10B10A2_UNORM);
	}

	FilmEffects::~FilmEffects()
	{
	}
	
	void FilmEffects::Render(Frame& CurrentFrame, uint32 SceneImageIndex, uint32 Width, uint32 Height)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.ComputeCommandList;

		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetComputeRootSignature(&Pipeline.RootSignature);

		Parameters.OutputImageIndex				= RenderTarget.ShaderResourceHandle.Index;
		Parameters.InputImageIndex				= SceneImageIndex;
		Parameters.EnableChromaticAberration	= static_cast<uint32>(Config::Get().bEnableChromaticAberration);
		Parameters.EnableLensDistortion			= static_cast<uint32>(Config::Get().bEnableLensDistortion);
		Parameters.EnableFilmGrain				= static_cast<uint32>(Config::Get().bEnableFilmGrain);

		commandList->PushComputeConstants(0, 6, &Parameters);

		const uint32 dispatchX = Math::RoundUp(Width  / 16u);
		const uint32 dispatchY = Math::RoundUp(Height / 16u);
		commandList->Dispatch(dispatchX, dispatchY, 1);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
	}

	void FilmEffects::Resize(uint32 Width, uint32 Height)
	{
		RenderTarget.Resize(Width, Height);
	}

	void FilmEffects::Release()
	{
	}
} // namespace Luden

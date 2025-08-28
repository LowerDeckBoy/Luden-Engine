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

	}

	SSR::~SSR()
	{
	}

	void SSR::Render(Frame* CurrentFrame, uint32 SceneImageIndex, uint32 NormalsIndex, uint32 RoughnessIndex, uint32 DepthIndex, SceneCamera* pCamera)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame->ComputeCommandList;

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
	}

	void SSR::Resize(uint32 Width, uint32 Height)
	{
	}

	void SSR::Release()
	{
	}
} // namespace Luden

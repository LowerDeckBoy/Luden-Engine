#include "Bloom.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	Bloom::Bloom(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: m_D3D12RHI(pD3D12RHI)
	{
		CreatePipelines(pShaderCompiler);

		RenderTarget.Create(m_D3D12RHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	Bloom::~Bloom()
	{
	}

	void Bloom::Render(Frame& CurrentFrame, uint32 BaseColorImage, uint32 LightPassImageIndex, uint32 SceneImageIndex)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;
		
		commandList->SetPipelineState(&BloomPSO.PipelineState);
		commandList->SetComputeRootSignature(&BloomPSO.RootSignature);
		commandList->ClearRenderTarget(RenderTarget.RenderTargetHandle, RenderTargetClearColor);
		struct
		{
			uint32 BaseColor;
			uint32 LightImage;
			uint32 SceneImage;
			uint32 pad;
		} constants{
			.BaseColor = BaseColorImage,
			.LightImage = LightPassImageIndex,
			.SceneImage = SceneImageIndex,
			.pad = 0
		};

		commandList->GetHandle()->SetComputeRoot32BitConstants(0, 4, &constants, 0);

		commandList->Dispatch(Math::RoundUp<uint32>((uint32)RenderTarget.GetDesc().Width / 32), Math::RoundUp<uint32>(RenderTarget.GetDesc().Height / 32), 1);

	}

	void Bloom::Resize(uint32 Width, uint32 Height)
	{
		RenderTarget.Resize(Width, Height);
	}

	void Bloom::CreatePipelines(ShaderCompiler* pShaderCompiler)
	{
		BloomPSO.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/Bloom.hlsl", true);
		VERIFY_D3D12_RESULT(BloomPSO.RootSignature.BuildFromShader(m_D3D12RHI->Device, &BloomPSO.Compute, PipelineType::Compute));

		D3D12ComputePipelineStateBuilder builder(m_D3D12RHI->Device);
		builder.SetComputeShader(&BloomPSO.Compute);
		builder.SetRootSignature(&BloomPSO.RootSignature);
		VERIFY_D3D12_RESULT(builder.Build(m_D3D12RHI->Device, BloomPSO));
		
	}
} // namespace Luden

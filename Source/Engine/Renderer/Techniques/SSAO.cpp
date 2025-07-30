#include "Asset/ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"
#include "SSAO.hpp"
#include <random>
#include "GeometryPass.hpp"
#include "Scene/SceneCamera.hpp"

namespace Luden
{
	SSAO::SSAO(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{
		RenderTarget.Create(m_RHI->Device, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT);

		CreatePipelines(pShaderCompiler);

		std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		
		for (uint32 i = 0; i < 64u; ++i)
		{
			DirectX::XMVECTOR sample = DirectX::XMVectorSet(
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator),
				0.0f
			);
			
			sample = DirectX::XMVector4Normalize(sample);
			const auto random = randomFloats(generator);
			sample.m128_f32[0] *= random;
			sample.m128_f32[1] *= random;
			sample.m128_f32[2] *= random;

			float scale = static_cast<float>(i) / 64.0f;
			scale = Math::Lerp(0.1f, 1.0f, scale * scale);
			sample.m128_f32[0] *= scale;
			sample.m128_f32[1] *= scale;
			sample.m128_f32[2] *= scale;

			DirectX::XMStoreFloat4(&Parameters.Samples[i], sample);
		}

		for (uint32 i = 0; i < 16; ++i)
		{
			Parameters.Noise[i].x = randomFloats(generator) * 2.0f - 1.0f;
			Parameters.Noise[i].y = randomFloats(generator) * 2.0f - 1.0f;
			Parameters.Noise[i].z = 0.0f;
		}

		ConstantBuffer = new D3D12ConstantBuffer(m_RHI->Device, &Parameters, sizeof(Parameters));

	}

	SSAO::~SSAO()
	{
		RenderTarget.Release();
		ConstantBuffer->Release();
		
	}

	void SSAO::Render(Frame& CurrentFrame, GeometryPass* pGBuffer, SceneCamera* pCamera)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;
		
		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetRootSignature(&Pipeline.RootSignature);

		commandList->ResourceTransition(&RenderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		//Parameters.Projection			= pCamera->GetProjection();
		Parameters.Projection			= pCamera->GetViewProjection();
		Parameters.OutputImageIndex		= RenderTarget.ShaderResourceHandle.Index;
		Parameters.BaseColorIndex		= pGBuffer->BaseColor.ShaderResourceHandle.Index;
		Parameters.NormalIndex			= pGBuffer->Normal.ShaderResourceHandle.Index;
		Parameters.WorldPositionIndex	= pGBuffer->WorldPosition.ShaderResourceHandle.Index;
		
		ConstantBuffer->Update(&Parameters);
		commandList->GetHandle()->SetComputeRootConstantBufferView(0, ConstantBuffer->GetBuffer()->GetGPUVirtualAddress());

		const uint32 dispatchX =  Math::RoundUp<uint32>((uint32)RenderTarget.GetDesc().Width / 8);
		const uint32 dispatchY =  Math::RoundUp<uint32>(RenderTarget.GetDesc().Height / 8);
		commandList->Dispatch(dispatchX, dispatchY, 1);
		commandList->ResourceTransition(&RenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	void SSAO::Resize(uint32 Width, uint32 Height)
	{
		RenderTarget.Resize(Width, Height);
	}

	void SSAO::Release()
	{
	}

	void SSAO::CreatePipelines(ShaderCompiler* pShaderCompiler)
	{
		Pipeline.Compute = pShaderCompiler->CompileCS("../../Shaders/Compute/SSAO.hlsl", true);

		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(m_RHI->Device, &Pipeline.Compute, PipelineType::Compute));

		D3D12ComputePipelineStateBuilder builder;
		builder.SetComputeShader(&Pipeline.Compute);
		builder.SetRootSignature(&Pipeline.RootSignature);
		VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, Pipeline));

	}
} // namespace Luden

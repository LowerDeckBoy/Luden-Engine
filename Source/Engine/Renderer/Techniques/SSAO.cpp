#include "Asset/ShaderCompiler.hpp"
#include "Asset/AssetImporter.hpp"
#include "D3D12/D3D12Utility.hpp"
#include "D3D12/D3D12Memory.hpp"
#include "SSAO.hpp"
#include <random>
#include "GeometryPass.hpp"
#include "Scene/SceneCamera.hpp"

namespace Luden
{
	SSAO::SSAO(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{
		// SSAO PSO
		{
			Pipeline.Compute = pShaderCompiler->CompileCS("../../Shaders/AO/SSAO.hlsl", true);

			VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(m_RHI->Device, &Pipeline.Compute, PipelineType::Compute));

			D3D12ComputePipelineStateBuilder builder;
			builder.SetComputeShader(&Pipeline.Compute);
			builder.SetRootSignature(&Pipeline.RootSignature);
			VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, Pipeline));
		}
		
		// Blur PSO
		{
			m_BlurPSO.Compute = pShaderCompiler->CompileCS("../../Shaders/AO/Blur.hlsl", true);

			VERIFY_D3D12_RESULT(m_BlurPSO.RootSignature.BuildFromShader(m_RHI->Device, &m_BlurPSO.Compute, PipelineType::Compute));

			D3D12ComputePipelineStateBuilder builder;
			builder.SetComputeShader(&m_BlurPSO.Compute);
			builder.SetRootSignature(&m_BlurPSO.RootSignature);
			VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, m_BlurPSO));
		}

		SSAORenderTarget.Create(m_RHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);
		BlurRenderTarget.Create(m_RHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);

		std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
		std::default_random_engine generator;
		
		for (uint32 i = 0; i < KernelSize; ++i)
		{
			DirectX::XMVECTOR sample = DirectX::XMVectorSet(
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator) * 2.0f - 1.0f,
				randomFloats(generator),
				0.0f
			);
			
			sample = DirectX::XMVector4Normalize(sample);
			const float random = randomFloats(generator);
			sample.m128_f32[0] *= random;
			sample.m128_f32[1] *= random;
			sample.m128_f32[2] *= random;

			float scale = static_cast<float>(i) / static_cast<float>(KernelSize);
			scale = Math::Lerp(0.1f, 1.0f, scale * scale);
			sample.m128_f32[0] *= scale;
			sample.m128_f32[1] *= scale;
			sample.m128_f32[2] *= scale;

			DirectX::XMStoreFloat4(&Parameters.Samples[i], sample);
		}

		ConstantBuffer = new D3D12ConstantBuffer(m_RHI->Device, &Parameters, sizeof(Parameters));

	}

	SSAO::~SSAO()
	{
		Release();
	}

	void SSAO::Render(Frame& CurrentFrame, GeometryPass* pGBuffer, uint32 NoiseImageIndex, SceneCamera* pCamera)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.ComputeCommandList;

		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetRootSignature(&Pipeline.RootSignature);

		commandList->ResourceTransition(&SSAORenderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandList->ResourceTransition(&BlurRenderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		Parameters.Projection			= DirectX::XMMatrixTranspose(pCamera->GetProjection());
		Parameters.InvProjection		= DirectX::XMMatrixTranspose(pCamera->GetInversedProjection());

		Parameters.OutputImageIndex		= SSAORenderTarget.ShaderResourceHandle.Index;
		Parameters.NoiseIndex			= NoiseImageIndex;
		Parameters.NormalIndex			= pGBuffer->NormalVS.ShaderResourceHandle.Index;
		Parameters.DepthIndex			= m_RHI->SceneDepthBuffer->ShaderResourceHandle.Index;

		ConstantBuffer->Update(&Parameters);
		commandList->GetHandle()->SetComputeRootConstantBufferView(0, ConstantBuffer->GetBuffer()->GetGPUVirtualAddress());

		const uint32 dispatchX = Math::RoundUp<uint32>((uint32)SSAORenderTarget.GetDesc().Width  / 8u);
		const uint32 dispatchY = Math::RoundUp<uint32>((uint32)SSAORenderTarget.GetDesc().Height / 8u);
		commandList->Dispatch(dispatchX, dispatchY, 1);
		

		if (bBlurSSAO)
		{
			commandList->SetPipelineState(&m_BlurPSO.PipelineState);
			commandList->SetRootSignature(&m_BlurPSO.RootSignature);

			struct
			{
				uint32 TargetImageIndex;
				uint32 DepthIndex;
				uint32 NormalIndex;
				float  Sharpness;
				uint32 Direction;
			} constants
			{
				.TargetImageIndex = BlurRenderTarget.ShaderResourceHandle.Index,
				.DepthIndex = m_RHI->SceneDepthBuffer->ShaderResourceHandle.Index,
				.NormalIndex = SSAORenderTarget.ShaderResourceHandle.Index,
				//.NormalIndex = pGBuffer->NormalVS.ShaderResourceHandle.Index,
				.Sharpness = BlurSharpness,
				.Direction = 0
			};

			commandList->PushComputeConstants(0, 5, &constants);
			commandList->Dispatch(dispatchX, dispatchY, 1);

			constants.Direction = 1;
			constants.NormalIndex = BlurRenderTarget.ShaderResourceHandle.Index;

			//commandList->PushComputeConstants(0, 5, &constants);
			//commandList->Dispatch(dispatchX, dispatchY, 1);
		}
		
		commandList->ResourceTransition(&SSAORenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(&BlurRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);

	}

	void SSAO::Resize(uint32 Width, uint32 Height)
	{
		SSAORenderTarget.Resize(Width, Height);
		BlurRenderTarget.Resize(Width, Height);
	}

	void SSAO::Release()
	{
		SSAORenderTarget.Release();
		BlurRenderTarget.Release();
		ConstantBuffer->Release();
	}

} // namespace Luden

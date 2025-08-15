#include "Asset/ShaderCompiler.hpp"
#include "Bloom.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	Bloom::Bloom(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{
		CreatePipelines(pShaderCompiler);

		CreateTextures(Width, Height);

	}

	Bloom::~Bloom()
	{
	}

	void Bloom::Render(Frame& CurrentFrame, uint32 LightPassImageIndex, uint32 SceneImageIndex)
	{
		auto renderBeginTime = Time::GetTimestamp();

		constexpr uint32 DispatchGroup = 8;

		auto commandList = CurrentFrame.ComputeCommandList;

		Parameters.LightImage = LightPassImageIndex;
		Parameters.SceneImage = SceneImageIndex;

		commandList->SetComputeRootSignature(&BloomPSO.RootSignature);

		// Downsample
		{
			commandList->SetPipelineState(&DownsamplePSO.PipelineState);

			// First downsample is made for Scene image.
			// Others are looped based on previous downsample.
			auto& texture = DownsampleTextures.at(0);
			commandList->ResourceTransition(&texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			Parameters.MipIndex = texture.ShaderResourceHandle.Index;
			commandList->GetHandle()->SetComputeRoot32BitConstants(0, 8, &Parameters, 0);
			commandList->Dispatch(Math::RoundUp<uint32>((uint32)texture.GetDesc().Width / DispatchGroup), Math::RoundUp<uint32>(texture.GetDesc().Height / DispatchGroup), 1);
			commandList->ResourceTransition(&texture, D3D12_RESOURCE_STATE_GENERIC_READ);

			for (uint32 mip = 1; mip < NumDownsamples; ++mip)
			{
				auto& downsample = DownsampleTextures.at(mip);
				commandList->ResourceTransition(&downsample, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				Parameters.MipIndex = downsample.ShaderResourceHandle.Index;
				Parameters.LightImage = DownsampleTextures.at(static_cast<usize>(mip - 1)).ShaderResourceHandle.Index;
				
				commandList->GetHandle()->SetComputeRoot32BitConstants(0, 8, &Parameters, 0);

				const uint32 dispatchX = Math::RoundUp<uint32>((uint32)downsample.GetDesc().Width / DispatchGroup);
				const uint32 dispatchY = Math::RoundUp<uint32>(downsample.GetDesc().Height / DispatchGroup);
				commandList->Dispatch(dispatchX, dispatchY, 1);
				commandList->ResourceTransition(&downsample, D3D12_RESOURCE_STATE_GENERIC_READ);
			}
		}

		// Blur
		{
			commandList->SetPipelineState(&BloomPSO.PipelineState);
			auto& texture = DownsampleTextures.at(static_cast<usize>(NumDownsamples - 1));
			commandList->ResourceTransition(&texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			Parameters.MipIndex = texture.ShaderResourceHandle.Index;
			Parameters.LightImage = texture.ShaderResourceHandle.Index;

			commandList->GetHandle()->SetComputeRoot32BitConstants(0, 8, &Parameters, 0);
		
			const uint32 dispatchX = Math::RoundUp<uint32>((uint32)texture.GetDesc().Width / DispatchGroup);
			const uint32 dispatchY = Math::RoundUp<uint32>(texture.GetDesc().Height / DispatchGroup);
			commandList->Dispatch(dispatchX, dispatchY, 1);
			commandList->ResourceTransition(&texture, D3D12_RESOURCE_STATE_GENERIC_READ);
		}

		// Upsample
		{
			// First upsamples in based on last downsample.
			// Others are looped based on previous upsample.
			auto& texture = UpsampleTextures.at(0);
			commandList->ResourceTransition(&texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			Parameters.MipIndex = texture.ShaderResourceHandle.Index;
			Parameters.LightImage = DownsampleTextures.at(static_cast<usize>(NumDownsamples - 1)).ShaderResourceHandle.Index;
			commandList->GetHandle()->SetComputeRoot32BitConstants(0, 8, &Parameters, 0);
			commandList->Dispatch(Math::RoundUp<uint32>((uint32)texture.GetDesc().Width / DispatchGroup), Math::RoundUp<uint32>(texture.GetDesc().Height / DispatchGroup), 1);
			commandList->ResourceTransition(&texture, D3D12_RESOURCE_STATE_GENERIC_READ);

			commandList->SetPipelineState(&UpsamplePSO.PipelineState);
			for (uint32 mip = 1; mip < NumUpsamples; ++mip)
			{
				auto& upsample = UpsampleTextures.at(static_cast<usize>(mip));
				commandList->ResourceTransition(&upsample, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				// Current Upsample texture
				Parameters.MipIndex = upsample.ShaderResourceHandle.Index;
				// 
				Parameters.LightImage = UpsampleTextures.at(static_cast<usize>(mip - 1)).ShaderResourceHandle.Index;

				commandList->GetHandle()->SetComputeRoot32BitConstants(0, 8, &Parameters, 0);

				const uint32 dispatchX = Math::RoundUp<uint32>((uint32)upsample.GetDesc().Width / DispatchGroup);
				const uint32 dispatchY = Math::RoundUp<uint32>(upsample.GetDesc().Height / DispatchGroup);
				commandList->Dispatch(dispatchX, dispatchY, 1);
				commandList->ResourceTransition(&upsample, D3D12_RESOURCE_STATE_GENERIC_READ);
			}
		}

		// For debug usage.
		commandList->ResourceTransition({
			{ &RenderTarget, D3D12_RESOURCE_STATE_COPY_DEST },
			{ &UpsampleTextures.at(5), D3D12_RESOURCE_STATE_COPY_SOURCE},
			});
		commandList->CopyResource(&UpsampleTextures.at(5), &RenderTarget);
		commandList->ResourceTransition({
			{ &RenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &UpsampleTextures.at(5), D3D12_RESOURCE_STATE_GENERIC_READ },
			});

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime).count();
	}

	void Bloom::Resize(uint32 Width, uint32 Height)
	{
		RenderTarget.Resize(Width, Height);

		// Resize all mip chain to appropriate dimensions.
		float previousWidth = static_cast<float>(Width);
		float previousHeight = static_cast<float>(Height);

		for (uint32 mip = 0; mip < NumDownsamples; ++mip)
		{
			previousWidth  = previousWidth  * 0.5f;
			previousHeight = previousHeight * 0.5f;
			DownsampleTextures.at(mip).Resize(static_cast<uint32>(previousWidth), static_cast<uint32>(previousHeight));
		}

		for (uint32 mip = 0; mip < NumUpsamples; ++mip)
		{
			UpsampleTextures.at(mip).Resize(static_cast<uint32>(previousWidth), static_cast<uint32>(previousHeight));
			previousWidth  = previousWidth  * 2.0f;
			previousHeight = previousHeight * 2.0f;
		}
	}

	void Bloom::Combine(Frame& CurrentFrame, D3D12RenderTexture* pSceneImage, uint32 ImageIndex)
	{
		constexpr uint32 DispatchGroup = 8;
		auto commandList = CurrentFrame.ComputeCommandList;

		commandList->SetPipelineState(&CombinePSO.PipelineState);
		commandList->SetComputeRootSignature(&BloomPSO.RootSignature);

		Parameters.MipIndex		= RenderTarget.ShaderResourceHandle.Index;
		Parameters.SceneImage	= pSceneImage->ShaderResourceHandle.Index;
		Parameters.BaseColor	= ImageIndex;

		commandList->GetHandle()->SetComputeRoot32BitConstants(0, 8, &Parameters, 0);
		commandList->Dispatch(Math::RoundUp<uint32>((uint32)pSceneImage->GetDesc().Width / DispatchGroup), Math::RoundUp<uint32>(pSceneImage->GetDesc().Height / DispatchGroup), 1);
	}

	void Bloom::Release()
	{
	}

	void Bloom::CreatePipelines(ShaderCompiler* pShaderCompiler)
	{
		// Bloom
		{
			BloomPSO.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/Bloom/Bloom.hlsl", true);
			VERIFY_D3D12_RESULT(BloomPSO.RootSignature.BuildFromShader(m_RHI->Device, &BloomPSO.Compute, PipelineType::Compute));

			D3D12ComputePipelineStateBuilder builder;
			builder.SetComputeShader(&BloomPSO.Compute);
			builder.SetRootSignature(&BloomPSO.RootSignature);
			VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, BloomPSO));
		}

		// Downsample
		{
			DownsamplePSO.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/Bloom/Downsample.hlsl", false);

			D3D12ComputePipelineStateBuilder builder;
			builder.SetComputeShader(&DownsamplePSO.Compute);
			builder.SetRootSignature(&BloomPSO.RootSignature);
			VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, DownsamplePSO));
		}

		// Upsample
		{
			UpsamplePSO.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/Bloom/Upsample.hlsl", false);

			D3D12ComputePipelineStateBuilder builder;
			builder.SetComputeShader(&UpsamplePSO.Compute);
			builder.SetRootSignature(&BloomPSO.RootSignature);
			VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, UpsamplePSO));
		}
		
		// Combine
		{
			CombinePSO.Compute = pShaderCompiler->CompileCS("../../Shaders/PostProcess/Bloom/Combine.hlsl", false);

			D3D12ComputePipelineStateBuilder builder;
			builder.SetComputeShader(&CombinePSO.Compute);
			builder.SetRootSignature(&BloomPSO.RootSignature);
			VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, CombinePSO));
		}
	}

	void Bloom::CreateTextures(uint32 Width, uint32 Height)
	{
		// Base Texture
		RenderTarget.Create(m_RHI->Device, Width, Height, DXGI_FORMAT_R11G11B10_FLOAT);

		float previousWidth  = static_cast<float>(Width);
		float previousHeight = static_cast<float>(Height);

		for (uint32 mip = 0; mip < NumDownsamples; ++mip)
		{
			previousWidth  = previousWidth  * 0.5f;
			previousHeight = previousHeight * 0.5f;

			D3D12RenderTexture texture;
			texture.Create(m_RHI->Device, static_cast<uint32>(previousWidth), static_cast<uint32>(previousHeight), RenderTarget.GetFormat());
			texture.SetDebugName(std::format("Post-Process Bloom Downsample texture #{}", mip));

			DownsampleTextures.push_back(std::move(texture));
		}

		for (uint32 mip = 0; mip < NumUpsamples; ++mip)
		{
			D3D12RenderTexture texture;
			texture.Create(m_RHI->Device, static_cast<uint32>(previousWidth), static_cast<uint32>(previousHeight), RenderTarget.GetFormat());
			texture.SetDebugName(std::format("Post-Process Bloom Upsample texture #{}", mip));

			UpsampleTextures.push_back(std::move(texture));
			
			previousWidth  = previousWidth  * 2.0f;
			previousHeight = previousHeight * 2.0f;
		}

	}
} // namespace Luden

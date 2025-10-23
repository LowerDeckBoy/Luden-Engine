#include "Asset/ShaderCompiler.hpp"
#include "Scene/SceneCamera.hpp"
#include "Atmosphere.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	Atmosphere::Atmosphere(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{

		DebugRenderTarget.Create(pD3D12RHI->Device, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT, DefaultClearColor);

		Pipeline.Compute = pShaderCompiler->CompileCS("../../Shaders/Sky/Atmosphere.hlsl", true);
		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(pD3D12RHI->Device, &Pipeline.Compute, PipelineType::Compute));
		D3D12ComputePipelineStateBuilder builder;
		builder.SetComputeShader(&Pipeline.Compute);
		builder.SetRootSignature(&Pipeline.RootSignature);
		VERIFY_D3D12_RESULT(builder.Build(pD3D12RHI->Device, Pipeline));

		// Temp
		Parameters.ScatteringCoefficiencyRayleigh = DirectX::XMFLOAT3(5.802f * 1e-6f, 13.558f * 1e-6f, 33.100f * 1e-6f);
	}

	Atmosphere::~Atmosphere()
	{
	}

	void Atmosphere::Render(Frame& CurrentFrame, SceneCamera* pCamera, uint32 SceneImageIndex, uint32 WorldPositionIndex, uint32 DepthIndex, DirectX::XMFLOAT3 SunPosition)
	{
		const auto renderTimeBegin = Time::GetTimestamp();

		auto commandList = CurrentFrame.ComputeCommandList;

		commandList->SetPipelineState(&Pipeline.PipelineState);
		commandList->SetComputeRootSignature(&Pipeline.RootSignature);

		commandList->ResourceTransition(&DebugRenderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		Parameters.SceneImageIndex		= SceneImageIndex;
		Parameters.OutputImageIndex		= DebugRenderTarget.ShaderResourceHandle.Index;
		Parameters.WorldPositionIndex	= WorldPositionIndex;
		Parameters.DepthIndex			= DepthIndex;
		
		Parameters.SunPosition			= DirectX::XMFLOAT4(SunPosition.x, SunPosition.y, SunPosition.z, 1.0f);
		Parameters.CameraPosition		= DirectX::XMFLOAT4(pCamera->Position.x, pCamera->Position.y, pCamera->Position.z, 1.0f);


		commandList->PushComputeConstants(0, 23, &Parameters);

		const uint32 dispatchBlock = 16;
		const uint32 dispatchX = Math::RoundUp((uint32)DebugRenderTarget.GetDesc().Width  / dispatchBlock);
		const uint32 dispatchY = Math::RoundUp((uint32)DebugRenderTarget.GetDesc().Height / dispatchBlock);

		commandList->Dispatch(dispatchX, dispatchY, 1);

		commandList->ResourceTransition(&DebugRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderTimeBegin);
	}

	void Atmosphere::Resize(uint32 Width, uint32 Height)
	{
		DebugRenderTarget.Resize(Width, Height);
	}

	void Atmosphere::Release()
	{
	}

} // namespace Luden

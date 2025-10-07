#include "D3D12/D3D12RHI.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneCamera.hpp"
#include "Skybox.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	Skybox::Skybox(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler)
		: m_D3D12RHI(pD3D12RHI)
	{
		m_PSO.Vertex = pShaderCompiler->CompileVS("../../Shaders/Sky/Skybox.hlsl", true);
		m_PSO.Pixel  = pShaderCompiler->CompilePS("../../Shaders/Sky/Skybox.hlsl", false);

		VERIFY_D3D12_RESULT(m_PSO.RootSignature.BuildFromShader(pD3D12RHI->Device, &m_PSO.Vertex, PipelineType::Graphics));

		D3D12PipelineStateBuilder builder(pD3D12RHI->Device);
		builder.SetRootSignature(&m_PSO.RootSignature);
		builder.SetVertexShader(&m_PSO.Vertex);
		builder.SetPixelShader(&m_PSO.Pixel);
		builder.SetCullMode(D3D12_CULL_MODE_NONE);
		builder.EnableDepth(false);
		//builder.SetDepthFormat(DXGI_FORMAT_D32_FLOAT);
		builder.SetRenderTargetFormats({ DXGI_FORMAT_R32G32B32A32_FLOAT });
		VERIFY_D3D12_RESULT(builder.Build(m_PSO.PipelineState));

		DebugRenderTarget.Create(pD3D12RHI->Device, 1920, 1080, DXGI_FORMAT_R32G32B32A32_FLOAT, DefaultClearColor);

		std::array<uint32, 36> indices =
		{
			 0, 1, 2, 2, 3, 0,   // Front
			 1, 4, 7, 7, 2, 1,   // Right
			 4, 5, 6, 6, 7, 4,   // Back
			 5, 0, 3, 3, 6, 5,   // Left
			 5, 4, 1, 1, 0, 5,   // Top
			 3, 2, 7, 7, 6, 3    // Bottom
		};

		m_IndexBuffer = pD3D12RHI->Device->CreateBuffer(
			BufferDesc{
				.BufferUsage = BufferUsageFlag::Index,
				.Data = indices.data(),
				.NumElements= 36,
				.Stride = sizeof(uint32),
				.Size = 36 * sizeof(uint32),
			}
		);

		auto indexBuffer = m_D3D12RHI->Device->Buffers.at(m_IndexBuffer);
		D3D12UploadContext::UploadBuffer(indexBuffer, indexBuffer->GetBufferDesc().Size);
		D3D12UploadContext::Upload();

	}

	Skybox::~Skybox()
	{
	}

	void Skybox::Render(Frame& CurrentFrame, SceneCamera* pCamera, DirectX::XMFLOAT3 SunPosition)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->SetPipelineState(&m_PSO.PipelineState);
		commandList->SetRootSignature(&m_PSO.RootSignature);

		auto indexBuffer = m_D3D12RHI->Device->Buffers.at(m_IndexBuffer);
		commandList->SetIndexBuffer(indexBuffer);

		commandList->ResourceTransition(&DebugRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetRenderTargets(DebugRenderTarget.RenderTargetHandle);
		commandList->ClearRenderTarget(DebugRenderTarget.RenderTargetHandle, DefaultClearColor);

		m_Transform.Translation = pCamera->Position;
		m_Transform.Scale = DirectX::XMFLOAT3(50.0f, 50.0f, 50.0f);
		SkyConstants.World = World * DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&m_Transform.Scale)) *
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat4(&m_Transform.Rotation)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_Transform.Translation));
		SkyConstants.View		= pCamera->GetView();
		SkyConstants.Projection = DirectX::XMMatrixTranspose(pCamera->GetProjection());

		SkyParameters.CameraPosition = pCamera->Position;
		SkyParameters.SunPosition = SunPosition;

		commandList->PushConstants(0, 16, &SkyParameters);
		commandList->PushConstants(1, 48, &SkyConstants);

		commandList->DrawIndexed(36, 0, 0);
		commandList->ResourceTransition(&DebugRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
	}


} // namespace Luden

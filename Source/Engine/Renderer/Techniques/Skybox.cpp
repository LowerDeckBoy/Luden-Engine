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

		D3D12PipelineStateBuilder builder(pD3D12RHI->Device);
		builder.SetRootSignature(&m_PSO.RootSignature);
		builder.SetVertexShader(&m_PSO.Vertex);
		builder.SetPixelShader(&m_PSO.Pixel);
		builder.SetCullMode(D3D12_CULL_MODE_NONE);
		//builder.SetFillMode(D3D12_FILL_MODE_WIREFRAME);
		builder.EnableDepth(true);
		builder.SetDepthFunc(D3D12_COMPARISON_FUNC_EQUAL);
		builder.SetDepthFormat(DXGI_FORMAT_D32_FLOAT);
		builder.SetRenderTargetFormats({ DXGI_FORMAT_R32G32B32A32_FLOAT });
		VERIFY_D3D12_RESULT(builder.Build(m_PSO.PipelineState));

		m_SkydomePSO.Vertex = pShaderCompiler->CompileVS("../../Shaders/Sky/Skydome.hlsl", true);
		m_SkydomePSO.Pixel = pShaderCompiler->CompilePS("../../Shaders/Sky/Skydome.hlsl", false);
		builder.SetVertexShader(&m_SkydomePSO.Vertex);
		builder.SetPixelShader(&m_SkydomePSO.Pixel);
		VERIFY_D3D12_RESULT(builder.Build(m_SkydomePSO.PipelineState));

		// Shared RS
		VERIFY_D3D12_RESULT(m_PSO.RootSignature.BuildFromShader(pD3D12RHI->Device, &m_PSO.Vertex, PipelineType::Graphics));

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
				.NumElements = 36,
				.Stride = sizeof(uint32),
				.Size = 36 * sizeof(uint32),
			}
		);

		auto indexBuffer = m_D3D12RHI->Device->Buffers.at(m_IndexBuffer);
		D3D12UploadContext::UploadBuffer(indexBuffer, indexBuffer->GetBufferDesc().Size);
		D3D12UploadContext::Upload();

		BuildSkydome();

	}

	Skybox::~Skybox()
	{
		//m_SkydomePSO.
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
		m_Transform.Scale = DirectX::XMFLOAT3(100.0f, 100.0f, 100.0f);
		//SkyConstants.World = World * DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&m_Transform.Scale)) *
		//	DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat4(&m_Transform.Rotation)) *
		//	DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_Transform.Translation));
		//SkyConstants.World		= DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(pCamera->GetInversedView(), DirectX::XMMatrixTranspose(pCamera->GetInversedProjection())));
		SkyConstants.World		= (DirectX::XMMatrixMultiply(pCamera->GetInversedView(), DirectX::XMMatrixTranspose(pCamera->GetInversedProjection())));
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

	void Skybox::RenderSkydome(Frame& CurrentFrame, SceneCamera* pCamera, DirectX::XMFLOAT3 SunPosition)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->SetPipelineState(&m_SkydomePSO.PipelineState);
		commandList->SetRootSignature(&m_PSO.RootSignature);

		commandList->ResourceTransition(&DebugRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetRenderTargets(DebugRenderTarget.RenderTargetHandle);
		commandList->ClearRenderTarget(DebugRenderTarget.RenderTargetHandle, DefaultClearColor);

		m_Transform.Translation = pCamera->Position;
		//m_Transform.Rotation = DirectX::XMFLOAT4(90.0f, 0.0f, 0.0f, 1.0f);
		//m_Transform.Scale = DirectX::XMFLOAT3(5.0f, 5.0f, 5.0f);
		SkyConstants.World = World * DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&m_Transform.Scale)) *
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat4(&m_Transform.Rotation)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_Transform.Translation));
		//SkyConstants.World = (DirectX::XMMatrixMultiply(pCamera->GetInversedView(), DirectX::XMMatrixTranspose(pCamera->GetInversedProjection())));
		SkyConstants.World = DirectX::XMMatrixTranspose(SkyConstants.World * (pCamera->GetViewProjection()));
		SkyConstants.View = (DirectX::XMMatrixMultiply(pCamera->GetInversedView(), DirectX::XMMatrixTranspose(pCamera->GetInversedProjection())));
		//SkyConstants.View = pCamera->GetView();
		SkyConstants.Projection = DirectX::XMMatrixTranspose(pCamera->GetProjection());

		SkyParameters.CameraPosition = pCamera->Position;
		SkyParameters.SunPosition = SunPosition;
		SkyParameters.VertexBufferIndex = SkydomeVertexBuffer.ShaderResourceView.Index;

		commandList->PushConstants(0, 16, &SkyParameters);
		commandList->PushConstants(1, 48, &SkyConstants);

		commandList->SetVertexBuffer(&SkydomeVertexBuffer);
		commandList->SetIndexBuffer(&SkydomeIndexBuffer);
		commandList->DrawIndexed(static_cast<uint32>(m_SkydomeIndices.size()), 0, 0);
		commandList->ResourceTransition(&DebugRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
	}

	void Skybox::BuildSkydome()
	{
		const uint32 radius = 4;
		const uint32 latitude = 64;
		const uint32 longitude = 64;

		const float deltaLatitude = Math::PI / latitude;
		const float deltaLongitude = 2.0f * Math::PI / longitude;
		const float lengthInv = 1.0f / radius;

		for (uint32 i = 0; i <= latitude; ++i)
		{
			float angle = Math::PI / 2.0f - i * deltaLatitude;
			float x = radius * std::cosf(angle);
			float y = x;
			float z = radius * std::sinf(angle);

			for (uint32 j = 0; j <= longitude; ++j)
			{
				float longitudeAngle = j * deltaLongitude;

				SphereVertex vertex{};
				vertex.Position.x = x * std::cosf(longitudeAngle);
				vertex.Position.y = y * std::sinf(longitudeAngle);
				vertex.Position.z = z;

				vertex.TexCoord.x = (float)j / (float)longitude;
				vertex.TexCoord.y = (float)i / (float)latitude;

				vertex.Normal.x = vertex.Position.x * lengthInv;
				vertex.Normal.y = vertex.Position.y * lengthInv;
				vertex.Normal.z = vertex.Position.z * lengthInv;

				m_SkydomeVertices.push_back(vertex);
			}
		}

		uint32 k1, k2;
		for (int i = 0; i < latitude; ++i)
		{
			k1 = i * (longitude + 1);
			k2 = k1 + longitude + 1;
			
			for (int j = 0; j < longitude; ++j, ++k1, ++k2)
			{
				if (i != 0)
				{
					m_SkydomeIndices.push_back(k1);
					m_SkydomeIndices.push_back(k2);
					m_SkydomeIndices.push_back(k1 + 1);
				}

				if (i != (latitude - 1))
				{
					m_SkydomeIndices.push_back(k1 + 1);
					m_SkydomeIndices.push_back(k2);
					m_SkydomeIndices.push_back(k2 + 1);
				}
			}
		}
		
		SkydomeVertexBuffer.Create(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::Vertex,
			.Data = m_SkydomeVertices.data(),
			.NumElements = static_cast<uint32>(m_SkydomeVertices.size()),
			.Stride = sizeof(SphereVertex),
			.bBindless = true
			});

		SkydomeIndexBuffer.Create(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::Index,
			.Data = m_SkydomeIndices.data(),
			.NumElements = static_cast<uint32>(m_SkydomeIndices.size()),
			.Stride = sizeof(uint32),
			.bBindless = true
			});

		D3D12UploadContext::UploadBuffer(&SkydomeVertexBuffer, SkydomeVertexBuffer.GetBufferDesc().Size);
		D3D12UploadContext::UploadBuffer(&SkydomeIndexBuffer, SkydomeIndexBuffer.GetBufferDesc().Size);
		D3D12UploadContext::Upload();

	}

	ProceduralSky::ProceduralSky(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler)
		: m_D3D12RHI(pD3D12RHI)
	{
		
		m_PSO.Vertex	= pShaderCompiler->CompileVS("../../Shaders/Sky/ProceduralSky.hlsl", true);
		m_PSO.Pixel		= pShaderCompiler->CompilePS("../../Shaders/Sky/ProceduralSky.hlsl", false);

		D3D12PipelineStateBuilder builder(pD3D12RHI->Device);
		builder.SetRootSignature(&m_PSO.RootSignature);
		builder.SetVertexShader(&m_PSO.Vertex);
		builder.SetPixelShader(&m_PSO.Pixel);
		builder.SetCullMode(D3D12_CULL_MODE_NONE);
		builder.EnableDepth(false);
		builder.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS);
		builder.SetDepthFormat(DXGI_FORMAT_D32_FLOAT);
		builder.SetRenderTargetFormats({ DXGI_FORMAT_R32G32B32A32_FLOAT });
		//builder.SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB });

		VERIFY_D3D12_RESULT(m_PSO.RootSignature.BuildFromShader(pD3D12RHI->Device, &m_PSO.Vertex, PipelineType::Graphics));
		VERIFY_D3D12_RESULT(builder.Build(pD3D12RHI->Device, m_PSO));

		//DebugRenderTarget.Create(pD3D12RHI->Device, 1920, 1080, DXGI_FORMAT_R32G32B32A32_FLOAT, DefaultClearColor);
		DebugRenderTarget.Create(pD3D12RHI->Device, 1920, 1080, DXGI_FORMAT_R32G32B32A32_FLOAT, DefaultClearColor);

		Initialize();
	}

	ProceduralSky::~ProceduralSky()
	{

	}

	void ProceduralSky::Initialize(uint32 VerticalCount, uint32 HorizontalCount)
	{
		for (uint32 i = 0; i < VerticalCount; i++)
		{
			for (uint32 j = 0; j < HorizontalCount; j++)
			{
				SkyVertex v{};
				v.Position.x = float(j) / (HorizontalCount - 1) * 2.0f - 1.0f;
				v.Position.y = float(i) / (VerticalCount - 1) * 2.0f - 1.0f;
				m_Vertices.push_back(v);
			}
		}

		m_Indices.reserve(static_cast<usize>((VerticalCount - 1) * (HorizontalCount - 1) * 6));

		for (uint32 i = 0; i < VerticalCount - 1; i++)
		{
			for (uint32 j = 0; j < HorizontalCount - 1; j++)
			{

				m_Indices.push_back((uint32)(j + 0 + HorizontalCount * (i + 0)));
				m_Indices.push_back((uint32)(j + 1 + HorizontalCount * (i + 0)));
				m_Indices.push_back((uint32)(j + 0 + HorizontalCount * (i + 1)));
				
				m_Indices.push_back((uint32)(j + 1 + HorizontalCount * (i + 0)));
				m_Indices.push_back((uint32)(j + 1 + HorizontalCount * (i + 1)));
				m_Indices.push_back((uint32)(j + 0 + HorizontalCount * (i + 1)));

			}
		}
	
		m_VertexBuffer.Create(m_D3D12RHI->Device, BufferDesc{
				.BufferUsage = BufferUsageFlag::Structured,
				.Data = m_Vertices.data(),
				.NumElements = static_cast<uint32>(m_Vertices.size()),
				.Stride = sizeof(m_Vertices.at(0)),
				.bBindless = true
			});

		m_IndexBuffer.Create(m_D3D12RHI->Device, BufferDesc{
				.BufferUsage = BufferUsageFlag::Index,
				.Data = m_Indices.data(),
				.NumElements = static_cast<uint32>(m_Indices.size()),
				.Stride = sizeof(m_Indices.at(0)),
				.bBindless = true
			});

		D3D12UploadContext::UploadBuffer(&m_VertexBuffer, m_VertexBuffer.GetBufferDesc().Size);
		D3D12UploadContext::UploadBuffer(&m_IndexBuffer, m_IndexBuffer.GetBufferDesc().Size);
		D3D12UploadContext::Upload();

	}

	void ProceduralSky::Render(Frame& CurrentFrame, SceneCamera* pCamera, DirectX::XMFLOAT3 /* SunPosition */, D3D12RenderTexture* pRenderTarget)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->SetPipelineState(&m_PSO.PipelineState);
		commandList->SetRootSignature(&m_PSO.RootSignature);

		if (pRenderTarget)
		{
			commandList->ResourceTransition(pRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->SetRenderTargets(pRenderTarget->RenderTargetHandle, m_D3D12RHI->SceneDepthBuffer->DepthStencilHandle);
			//commandList->ClearRenderTarget(RenderTarget.RenderTargetHandle, DefaultClearColor);
		}
		else
		{
			commandList->ResourceTransition(&DebugRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->SetRenderTargets(DebugRenderTarget.RenderTargetHandle);
			commandList->ClearRenderTarget(DebugRenderTarget.RenderTargetHandle, DefaultClearColor);
		}	

		SkyConstants.InversedViewProjection = DirectX::XMMatrixTranspose(pCamera->GetInversedView()) * DirectX::XMMatrixTranspose(pCamera->GetInversedProjection());

		SkyParameters.CameraPosition = pCamera->Position;
		SkyParameters.VertexBufferIndex = m_VertexBuffer.ShaderResourceView.Index;

		commandList->PushConstants(0, 18, &SkyParameters);
		commandList->PushConstants(1, 16, &SkyConstants);
		
		commandList->SetVertexBuffer(&m_VertexBuffer);
		commandList->SetIndexBuffer(&m_IndexBuffer);
		commandList->DrawIndexed(m_IndexBuffer.GetBufferDesc().NumElements, 0, 0);
		//commandList->Draw(3);


		commandList->ResourceTransition((pRenderTarget ? pRenderTarget : &DebugRenderTarget), D3D12_RESOURCE_STATE_GENERIC_READ);
		//commandList->ResourceTransition(&RenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
		//commandList->ResourceTransition(&DebugRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
		
	}

	void ProceduralSky::Resize(uint32 Width, uint32 Height)
	{
		DebugRenderTarget.Resize(Width, Height);
	}

	SkyTest::SkyTest(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
	{
		m_D3D12RHI = pD3D12RHI;


		// Transmittance
		{
			//TransmittanceTexture = new D3D12RenderTexture(m_D3D12RHI->Device, 256, 64, DXGI_FORMAT_R32G32B32A32_FLOAT, DefaultClearColor, "Sky Transmittance Texture");
			TransmittanceTexture = new D3D12RenderTexture(m_D3D12RHI->Device, 256, 64, DXGI_FORMAT_R16G16B16A16_FLOAT, DefaultClearColor, "Sky Transmittance Texture");

			m_TransmittancePSO.Compute = pShaderCompiler->CompileCS("../../Shaders/SkyTest/Transmittance.hlsl", true);
			m_TransmittancePSO.RootSignature.BuildFromShader(m_D3D12RHI->Device, &m_TransmittancePSO.Compute, PipelineType::Compute);
			D3D12ComputePipelineStateBuilder builder;
			builder.SetComputeShader(&m_TransmittancePSO.Compute);
			builder.SetRootSignature(&m_TransmittancePSO.RootSignature);
			VERIFY_D3D12_RESULT(builder.Build(m_D3D12RHI->Device, m_TransmittancePSO));
		}

		// Multi-scattering
		{

		}
	}

	SkyTest::~SkyTest()
	{
	}

	void SkyTest::Render(Frame& CurrentFrame, SceneCamera* /* pCamera */, DirectX::XMFLOAT3 /* SunPosition */)
	{

	}

	void SkyTest::Resize(uint32 Width, uint32 Height)
	{
		//TransmittanceTexture->Resize(Width, Height);
	}

	void SkyTest::PrecomputeTransmittance(Frame& CurrentFrame)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->ResourceTransition(TransmittanceTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetPipelineState(&m_TransmittancePSO.PipelineState);
		commandList->SetRootSignature(&m_TransmittancePSO.RootSignature);

		PushConstants.TransmittanceIndex = TransmittanceTexture->ShaderResourceHandle.Index;
		commandList->PushComputeConstants(0, 1, &PushConstants);

		const uint32 dispatchX = 256 / 8;
		const uint32 dispatchY = 64  / 4;
		commandList->Dispatch(dispatchX, dispatchY, 1);

		commandList->ResourceTransition(TransmittanceTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

} // namespace Luden

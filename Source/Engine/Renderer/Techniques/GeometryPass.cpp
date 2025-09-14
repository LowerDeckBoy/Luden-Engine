#include "Scene/Scene.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "GeometryPass.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	GeometryPass::GeometryPass(D3D12RHI* pRHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pRHI)
	{
		Initialize(pRHI, Width, Height);
		CreatePipeline(pShaderCompiler);
	}

	GeometryPass::~GeometryPass()
	{
		Release();
	}

	void GeometryPass::Initialize(D3D12RHI* pRHI, uint32 Width, uint32 Height)
	{
		Release();

		BaseColor.Create(pRHI->Device, Width, Height,			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,	RenderTargetClearColor, "GBuffer BaseColor");
		Normal.Create(pRHI->Device, Width, Height,				DXGI_FORMAT_R16G16B16A16_FLOAT,		RenderTargetClearColor, "GBuffer Normal WorldSpace");
		NormalWS.Create(pRHI->Device, Width, Height,			DXGI_FORMAT_R16G16B16A16_FLOAT,		RenderTargetClearColor, "GBuffer Normal ViewSpace");
		MotionVectors.Create(pRHI->Device, Width, Height,		DXGI_FORMAT_R16G16B16A16_FLOAT,		RenderTargetClearColor, "GBuffer Motion Vectors");
		MetallicRoughness.Create(pRHI->Device, Width, Height,	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,	RenderTargetClearColor, "GBuffer MetallicRoughness");
		Emissive.Create(pRHI->Device, Width, Height,			DXGI_FORMAT_R16G16B16A16_FLOAT,		RenderTargetClearColor, "GBuffer Emissive");
		WorldPosition.Create(pRHI->Device, Width, Height,		DXGI_FORMAT_R16G16B16A16_FLOAT,		RenderTargetClearColor, "GBuffer WorldPosition");
		Depth.Create(pRHI->Device, Width, Height,				DXGI_FORMAT_R16G16B16A16_FLOAT,		RenderTargetClearColor, "GBuffer Depth");

		m_RenderTargetHandles.push_back(&BaseColor.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Normal.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&NormalWS.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&MotionVectors.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&MetallicRoughness.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Emissive.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&WorldPosition.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Depth.RenderTargetHandle);

	}

	void GeometryPass::CreatePipeline(ShaderCompiler* pShaderCompiler)
	{
		// Opaque PSO
		{
			Pipeline.Amplification = pShaderCompiler->CompileAS("../../Shaders/Mesh/GBuffer_AS.hlsl", false);
			Pipeline.Mesh = pShaderCompiler->CompileMS("../../Shaders/Mesh/GBuffer_MS.hlsl", true);
			Pipeline.Pixel = pShaderCompiler->CompilePS("../../Shaders/Mesh/GBuffer_PS.hlsl", false);

			VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(m_RHI->Device, &Pipeline.Mesh, PipelineType::Graphics));

			D3D12MeshPipelineStateBuilder builder(m_RHI->Device);
			builder.SetRootSignature(&Pipeline.RootSignature);
			builder.SetAmplificationShader(&Pipeline.Amplification);
			builder.SetMeshShader(&Pipeline.Mesh);
			builder.SetPixelShader(&Pipeline.Pixel);
			builder.EnableDepth(true);
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
			builder.SetAlphaModeOpaque(0);
			builder.SetAlphaModeOpaque(1);
			builder.SetAlphaModeOpaque(2);
			builder.SetAlphaModeOpaque(3);
			builder.SetAlphaModeOpaque(4);
			builder.SetAlphaModeOpaque(5);
			builder.SetAlphaModeOpaque(6);
			builder.SetAlphaModeOpaque(7);
			builder.SetRenderTargetFormats({
				BaseColor.GetFormat(),
				Normal.GetFormat(),
				NormalWS.GetFormat(),
				MotionVectors.GetFormat(),
				MetallicRoughness.GetFormat(),
				Emissive.GetFormat(),
				WorldPosition.GetFormat(),
				Depth.GetFormat()
				});

			VERIFY_D3D12_RESULT(builder.Build(Pipeline.PipelineState));
		}

		// Blend PSO
		{
			BlendPipelineState.Amplification = pShaderCompiler->CompileAS("../../Shaders/Mesh/GBufferBlend_MS.hlsl", false);
			BlendPipelineState.Mesh = pShaderCompiler->CompileMS("../../Shaders/Mesh/GBufferBlend_MS.hlsl", true);
			BlendPipelineState.Pixel = pShaderCompiler->CompilePS("../../Shaders/Mesh/GBufferBlend_MS.hlsl", false);
			//BlendPipelineState.Amplification = pShaderCompiler->CompileAS("../../Shaders/Mesh/GBuffer_AS.hlsl", false);
			//BlendPipelineState.Mesh = pShaderCompiler->CompileMS("../../Shaders/Mesh/GBuffer_MS.hlsl", true);
			//BlendPipelineState.Pixel = pShaderCompiler->CompilePS("../../Shaders/Mesh/GBuffer_PS.hlsl", false);

			VERIFY_D3D12_RESULT(BlendPipelineState.RootSignature.BuildFromShader(m_RHI->Device, &BlendPipelineState.Mesh, PipelineType::Graphics));

			D3D12MeshPipelineStateBuilder builder(m_RHI->Device);
			builder.SetRootSignature(&BlendPipelineState.RootSignature);
			builder.SetAmplificationShader(&BlendPipelineState.Amplification);
			builder.SetMeshShader(&BlendPipelineState.Mesh);
			builder.SetPixelShader(&BlendPipelineState.Pixel);
			builder.SetCullMode(D3D12_CULL_MODE_NONE);
			builder.SetAlphaModeBlend(0);
			builder.SetAlphaModeBlend(1);
			builder.SetAlphaModeBlend(2);
			builder.SetAlphaModeBlend(3);
			builder.SetAlphaModeBlend(4);
			builder.SetAlphaModeBlend(5);
			builder.SetAlphaModeBlend(6);
			builder.SetAlphaModeBlend(7);
			builder.SetAlphaBlendDepthDesc();
			builder.SetRenderTargetFormats({
				BaseColor.GetFormat(),
				Normal.GetFormat(),
				NormalWS.GetFormat(),
				MotionVectors.GetFormat(),
				MetallicRoughness.GetFormat(),
				Emissive.GetFormat(),
				WorldPosition.GetFormat(),
				Depth.GetFormat()
				});

			VERIFY_D3D12_RESULT(builder.Build(BlendPipelineState.PipelineState));
		}
		
		// Indirect Draw PSO
		{
			IndirectPipelineState.Compute = pShaderCompiler->CompileCS("../../Shaders/Indirect/Indirect.hlsl", true);

			VERIFY_D3D12_RESULT(IndirectPipelineState.RootSignature.BuildFromShader(m_RHI->Device, &IndirectPipelineState.Compute, PipelineType::Compute));

			D3D12ComputePipelineStateBuilder builder;
			builder.SetComputeShader(&IndirectPipelineState.Compute);
			builder.SetRootSignature(&IndirectPipelineState.RootSignature);
			VERIFY_D3D12_RESULT(builder.Build(m_RHI->Device, IndirectPipelineState));
		}
	}

	void GeometryPass::Release()
	{
		delete IndirectSignature;

		BaseColor.Release();
		Normal.Release();
		NormalWS.Release();
		MotionVectors.Release();
		MetallicRoughness.Release();
		Emissive.Release();
		WorldPosition.Release();
		Depth.Release();

		m_RenderTargetHandles.clear();
		m_RenderTargetHandles.shrink_to_fit();

	}

	void GeometryPass::Resize(uint32 Width, uint32 Height)
	{
		BaseColor.Resize(Width, Height);
		Normal.Resize(Width, Height);
		NormalWS.Resize(Width, Height);
		MotionVectors.Resize(Width, Height);
		MetallicRoughness.Resize(Width, Height);
		Emissive.Resize(Width, Height);
		WorldPosition.Resize(Width, Height);
		Depth.Resize(Width, Height);
	}

	void GeometryPass::Render(Scene* pScene, SceneCamera* pCamera, Frame& CurrentFrame)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->ResourceTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Normal,				D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &NormalWS,			D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &MotionVectors,		D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Emissive,			D3D12_RESOURCE_STATE_RENDER_TARGET }, 
			{ &WorldPosition,		D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Depth,				D3D12_RESOURCE_STATE_RENDER_TARGET },
		});

		commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTargets(m_RenderTargetHandles, RenderTargetClearColor);

		auto device = m_RHI->Device;

		commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&Pipeline.PipelineState);

		commandList->PushConstants(1, 28, &pScene->Consts);

		// Draw all opaque objects.
		for (auto& model : pScene->Models)
		{
			if (model->OpaqueMeshes.empty()) continue;

			// Works, but gotta rework it later.
			// It's gonna require changes inside of Editor hierarchy and selection, tho.
			// :(
			if (!model->GetComponent<ecs::NameComponent>().bVisibleInScene) continue;

			for (auto& mesh : model->OpaqueMeshes)
			{
				uint32 vertexBuffer				= device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
				uint32 meshletBuffer			= device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
				uint32 meshletVerticesBuffer	= device->Buffers.at(mesh.MeshletVerticesBuffer)->ShaderResourceView.Index;
				uint32 meshletTrianglesBuffer	= device->Buffers.at(mesh.MeshletTrianglesBuffer)->ShaderResourceView.Index;
				uint32 meshletBoundsBuffer		= device->Buffers.at(mesh.MeshletBoundsBuffer)->ShaderResourceView.Index;

				struct
				{
					uint32 vertex;
					uint32 meshlet;
					uint32 meshletVertices;
					uint32 meshletTriangles;
					uint32 meshletBounds;
					uint32 bDrawMeshlets;
					uint32 bMeshletCulling;
					uint32 bAlphaMask;
					uint32 transformsBuffer;
					uint32 materialBuffer;
					uint32 materialID;
					uint32 transformID;
					float  nearZ;
					float  farZ;
				} buffers
				{
					.vertex				= vertexBuffer,
					.meshlet			= meshletBuffer,
					.meshletVertices	= meshletVerticesBuffer,
					.meshletTriangles	= meshletTrianglesBuffer,
					.meshletBounds		= meshletBoundsBuffer,
					.bDrawMeshlets		= (uint32)Config::Get().bDrawMeshlets,
					.bMeshletCulling	= (uint32)Config::Get().bMeshletCulling,
					.bAlphaMask			= (uint32)Config::Get().bAlphaMask,
					.transformsBuffer	= pScene->TransformsBuffer->ShaderResourceView.Index,
					.materialBuffer		= pScene->MaterialBuffer->ShaderResourceView.Index,
					.materialID			= mesh.MaterialID,
					.transformID		= model->TransformID,
					.nearZ				= pCamera->zNear,
					.farZ				= pCamera->zFar
				};
			
				CurrentFrame.GraphicsCommandList->PushConstants(0, 14 , &buffers);

				CurrentFrame.GraphicsCommandList->DispatchMesh(Math::RoundUp<uint32>(mesh.NumMeshlets / 32), 1, 1);
			}
		}

		//commandList->ResourceTransition({
		//	{ &BaseColor,			D3D12_RESOURCE_STATE_GENERIC_READ },
		//	{ &Normal,				D3D12_RESOURCE_STATE_GENERIC_READ },
		//	{ &NormalWS,			D3D12_RESOURCE_STATE_GENERIC_READ },
		//	{ &MotionVectors,		D3D12_RESOURCE_STATE_GENERIC_READ },
		//	{ &MetallicRoughness,	D3D12_RESOURCE_STATE_GENERIC_READ },
		//	{ &Emissive,			D3D12_RESOURCE_STATE_GENERIC_READ },
		//	{ &WorldPosition,		D3D12_RESOURCE_STATE_GENERIC_READ },
		//	{ &Depth,				D3D12_RESOURCE_STATE_GENERIC_READ }
		//});

		RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);

	}

	void GeometryPass::RenderTransparent(Scene* pScene, SceneCamera* pCamera, Frame& CurrentFrame)
	{
		const auto renderBeginTime = Time::GetTimestamp();

		auto commandList = CurrentFrame.GraphicsCommandList;

		//commandList->ResourceTransition({
		//	{ &BaseColor,			D3D12_RESOURCE_STATE_RENDER_TARGET },
		//	{ &Normal,				D3D12_RESOURCE_STATE_RENDER_TARGET },
		//	{ &NormalWS,			D3D12_RESOURCE_STATE_RENDER_TARGET },
		//	{ &MotionVectors,		D3D12_RESOURCE_STATE_RENDER_TARGET },
		//	{ &MetallicRoughness,	D3D12_RESOURCE_STATE_RENDER_TARGET },
		//	{ &Emissive,			D3D12_RESOURCE_STATE_RENDER_TARGET },
		//	{ &WorldPosition,		D3D12_RESOURCE_STATE_RENDER_TARGET },
		//	{ &Depth,				D3D12_RESOURCE_STATE_RENDER_TARGET },
		//	});

		//commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		//commandList->ClearRenderTargets(m_RenderTargetHandles, RenderTargetClearColor);

		auto device = m_RHI->Device;

		commandList->SetRootSignature(&BlendPipelineState.RootSignature);
		commandList->SetPipelineState(&BlendPipelineState.PipelineState);

		commandList->PushConstants(1, 28, &pScene->Consts);

		// Draw all opaque objects.
		for (auto& model : pScene->Models)
		{
			if (model->BlendMeshes.empty()) continue;

			// Works, but gotta rework it later.
			// It's gonna require changes inside of Editor hierarchy and selection, tho.
			// :(
			if (!model->GetComponent<ecs::NameComponent>().bVisibleInScene) continue;

			for (auto& mesh : model->BlendMeshes)
			{
				uint32 vertexBuffer = device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
				uint32 meshletBuffer = device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
				uint32 meshletVerticesBuffer = device->Buffers.at(mesh.MeshletVerticesBuffer)->ShaderResourceView.Index;
				uint32 meshletTrianglesBuffer = device->Buffers.at(mesh.MeshletTrianglesBuffer)->ShaderResourceView.Index;
				uint32 meshletBoundsBuffer = device->Buffers.at(mesh.MeshletBoundsBuffer)->ShaderResourceView.Index;

				struct
				{
					uint32 vertex;
					uint32 meshlet;
					uint32 meshletVertices;
					uint32 meshletTriangles;
					uint32 meshletBounds;
					uint32 bDrawMeshlets;
					uint32 bMeshletCulling;
					uint32 bAlphaMask;
					uint32 transformsBuffer;
					uint32 materialBuffer;
					uint32 materialID;
					uint32 transformID;
					float  nearZ;
					float  farZ;
				} buffers
				{
					.vertex = vertexBuffer,
					.meshlet = meshletBuffer,
					.meshletVertices = meshletVerticesBuffer,
					.meshletTriangles = meshletTrianglesBuffer,
					.meshletBounds = meshletBoundsBuffer,
					.bDrawMeshlets = (uint32)Config::Get().bDrawMeshlets,
					.bMeshletCulling = (uint32)Config::Get().bMeshletCulling,
					.bAlphaMask = (uint32)Config::Get().bAlphaMask,
					.transformsBuffer = pScene->TransformsBuffer->ShaderResourceView.Index,
					.materialBuffer = pScene->MaterialBuffer->ShaderResourceView.Index,
					.materialID = mesh.MaterialID,
					.transformID = model->TransformID,
					.nearZ = pCamera->zNear,
					.farZ = pCamera->zFar
				};

				CurrentFrame.GraphicsCommandList->PushConstants(0, 14, &buffers);

				CurrentFrame.GraphicsCommandList->DispatchMesh(Math::RoundUp<uint32>(mesh.NumMeshlets / 32), 1, 1);
			}
		}

		commandList->ResourceTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Normal,				D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &NormalWS,			D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &MotionVectors,		D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Emissive,			D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &WorldPosition,		D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Depth,				D3D12_RESOURCE_STATE_GENERIC_READ }
			});

		//RenderTime = Time::GetDurationInMiliseconds(renderBeginTime);
	}

	void GeometryPass::RenderIndirect(Scene* /* pScene */, SceneCamera* /* pCamera */, Frame& CurrentFrame)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->ResourceTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Normal,				D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &MotionVectors,		D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Emissive,			D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &WorldPosition,		D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Depth,				D3D12_RESOURCE_STATE_RENDER_TARGET },
			});

		commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTargets(m_RenderTargetHandles, RenderTargetClearColor);

		//auto device = m_RHI->Device;

		//commandList->SetPipelineState(&Pipeline.PipelineState);
		//commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&IndirectPipelineState.PipelineState);
		commandList->SetRootSignature(&IndirectPipelineState.RootSignature);

		//commandList->PushConstants(1, 28, &pScene->Consts);

		commandList->ExecuteIndirect(IndirectSignature);

		commandList->ResourceTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Normal,				D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &MotionVectors,		D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Emissive,			D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &WorldPosition,		D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Depth,				D3D12_RESOURCE_STATE_GENERIC_READ },
			});
	}

} // namespace Luden

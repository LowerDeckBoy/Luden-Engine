#include "Scene/Scene.hpp"
#include "GeometryPass.hpp"

namespace Luden
{
	GeometryPass::GeometryPass(D3D12RHI* pRHI, uint32 Width, uint32 Height)
		: RenderPass(pRHI)
	{
		Initialize(pRHI, Width, Height);
	}

	GeometryPass::~GeometryPass()
	{
		Release();
	}

	void GeometryPass::Initialize(D3D12RHI* pRHI, uint32 Width, uint32 Height)
	{
		//Depth				= new D3D12RenderTexture(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);
		BaseColor			= new D3D12RenderTexture(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);
		Normal				= new D3D12RenderTexture(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);
		MetallicRoughness	= new D3D12RenderTexture(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);
		Emissive			= new D3D12RenderTexture(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);
		//WorldPosition		= new D3D12RenderTexture(pRHI->Device, Width, Height, DXGI_FORMAT_R32G32B32A32_FLOAT);

		//m_RenderTargetHandles.push_back(Depth->RenderTargetHandle);
		m_RenderTargetHandles.push_back(&BaseColor->RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Normal->RenderTargetHandle);
		m_RenderTargetHandles.push_back(&MetallicRoughness->RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Emissive->RenderTargetHandle);
		//m_RenderTargetHandles.push_back(WorldPosition->RenderTargetHandle);
	}

	void GeometryPass::Release()
	{
		//delete Depth;
		delete BaseColor;
		delete Normal;
		delete MetallicRoughness;
		delete Emissive;
		//delete WorldPosition;
	}

	void GeometryPass::Resize(uint32 Width, uint32 Height)
	{
		//Depth->Resize(Width, Height);
		BaseColor->Resize(Width, Height);
		Normal->Resize(Width, Height);
		MetallicRoughness->Resize(Width, Height);
		Emissive->Resize(Width, Height);
		//WorldPosition->Resize(Width, Height);
	}

	void GeometryPass::Render(Scene* pScene, Frame& CurrentFrame)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;
		auto device = m_RHI->Device;
		
		//commandList->ResourceTransition(Depth, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(BaseColor, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(Normal, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(MetallicRoughness, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(Emissive, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//commandList->ResourceTransition(WorldPosition, D3D12_RESOURCE_STATE_RENDER_TARGET);

		commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTargets(m_RenderTargetHandles);

		// Draw
		//CurrentFrame.GraphicsCommandList->SetRootSignature(&MeshTempRS);
		//CurrentFrame.GraphicsCommandList->SetPipelineState(&MeshTempPSO);

		for (auto& model : pScene->Models)
		{
			auto& transform = model.GetComponent<ecs::TransformComponent>();
			transform.Update();

			auto* constantBuffer = device->ConstantBuffers.at(model.ConstantBuffer);
			CurrentFrame.GraphicsCommandList->SetConstantBuffer(0, constantBuffer);

			for (auto& mesh : model.Meshes)
			{
				//if (!Camera->IsInsideFrustum(mesh.BoundingBox))
				//{
				//	CulledVertices += mesh.NumVertices;
				//	//continue;
				//}
				

				//mesh.Transform.Update();
				//model.cbObjectTransforms.WVP = DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix * Camera->GetViewProjection());
				//model.cbObjectTransforms.World = DirectX::XMMatrixTranspose(mesh.Transform.WorldMatrix * transform.WorldMatrix);
				//constantBuffer->Update(&model.cbObjectTransforms);

				uint32 vertexBuffer = device->Buffers.at(mesh.VertexBuffer)->ShaderResourceView.Index;
				uint32 meshletBuffer = device->Buffers.at(mesh.MeshletsBuffer)->ShaderResourceView.Index;
				uint32 meshletVerticesBuffer = device->Buffers.at(mesh.MeshletVerticesBuffer)->ShaderResourceView.Index;
				uint32 meshletTrianglesBuffer = device->Buffers.at(mesh.MeshletTrianglesBuffer)->ShaderResourceView.Index;
				uint32 meshletBoundsBuffer = device->Buffers.at(mesh.MeshletBoundsBuffer)->ShaderResourceView.Index;

				auto& material = model.Materials.at(mesh.MaterialId);

				struct
				{
					uint32 vertex;
					uint32 meshlet;
					uint32 meshletVertices;
					uint32 meshletTriangles;
					uint32 meshletBounds;
					uint32 bDrawMeshlets;
					uint32 bAlphaMask;
				} buffers
				{
					.vertex = vertexBuffer,
					.meshlet = meshletBuffer,
					.meshletVertices = meshletVerticesBuffer,
					.meshletTriangles = meshletTrianglesBuffer,
					.meshletBounds = meshletBoundsBuffer,
					.bDrawMeshlets = (uint32)Config::Get().bDrawMeshlets,
					.bAlphaMask = (uint32)Config::Get().bAlphaMask
				};

				CurrentFrame.GraphicsCommandList->PushConstants(1, 7, &buffers);
				CurrentFrame.GraphicsCommandList->PushConstants(2, 20, &material);

				//CurrentFrame.GraphicsCommandList->DispatchMesh(mesh.NumMeshlets, 1, 1);
				CurrentFrame.GraphicsCommandList->DispatchMesh(ROUND_UP(mesh.NumMeshlets / 32), 1, 1);
			}
		}

		//commandList->ResourceTransition(Depth, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(BaseColor, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(Normal, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(MetallicRoughness, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(Emissive, D3D12_RESOURCE_STATE_GENERIC_READ);
		//commandList->ResourceTransition(WorldPosition, D3D12_RESOURCE_STATE_GENERIC_READ);

	}

	void GeometryPass::Render(Scene* pScene, Frame& CurrentFrame, std::function<void()> const& DrawFunction)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;
		//auto device = m_RHI->Device;

		//commandList->ResourceTransition(Depth, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(BaseColor, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(Normal, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(MetallicRoughness, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(Emissive, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//commandList->ResourceTransition(WorldPosition, D3D12_RESOURCE_STATE_RENDER_TARGET);

		commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&Pipeline.PipelineState);

		commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTargets(m_RenderTargetHandles);

		DrawFunction();

		commandList->ResourceTransition(BaseColor, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(Normal, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(MetallicRoughness, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(Emissive, D3D12_RESOURCE_STATE_GENERIC_READ);

	}
} // namespace Luden

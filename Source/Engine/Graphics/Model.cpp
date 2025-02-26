#include "D3D12/D3D12Device.hpp"
#include "Model.hpp"
#include "D3D12/D3D12UploadContext.hpp"

namespace Luden
{
	void Model::Create(D3D12Device* pDevice)
	{
		//pScene->CreateEntity(*this);

		ConstantBuffer = pDevice->CreateConstantBuffer(&cbPerObject, sizeof(cbPerObject));

		for (auto& mesh : Meshes)
		{
			mesh.VertexBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Structured,
				.Data			= mesh.Vertices.data(),
				.NumElements	= mesh.NumVertices,
				.Stride			= sizeof(Vertex),
				.bBindless		= true
				});

			mesh.IndexBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Index,
				.Data			= mesh.Indices.data(),
				.NumElements	= mesh.NumIndices,
				.Stride			= sizeof(uint32),
				.bBindless		= false
				});

			mesh.IndexBufferView = D3D12_INDEX_BUFFER_VIEW{
				.BufferLocation = pDevice->Buffers.at(mesh.IndexBuffer)->GetHandleRaw()->GetGPUVirtualAddress(),
				.SizeInBytes	= mesh.NumIndices * (uint32)sizeof(uint32),
				.Format			= DXGI_FORMAT_R32_UINT
			};

			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.VertexBuffer), mesh.NumVertices * (uint64)sizeof(Vertex));
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.IndexBuffer), mesh.NumIndices * (uint64)sizeof(uint32));

			mesh.MeshletsBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Structured,
				.Data			= mesh.Meshlets.data(),
				.NumElements	= mesh.NumMeshlets,
				.Stride			= sizeof(DirectX::Meshlet),
				.bBindless		= true
				});

			mesh.MeshletsVerticesBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Structured,
				.Data			= mesh.MeshletVertices.data(),
				.NumElements	= (uint32)mesh.MeshletVertices.size(),
				.Stride			= sizeof(uint8),
				.bBindless		= true
				});

			mesh.MeshletsTrianglesBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Structured,
				.Data			= mesh.MeshletTriangles.data(),
				.NumElements	= (uint32)mesh.MeshletTriangles.size(),
				.Stride			= sizeof(DirectX::MeshletTriangle),
				.bBindless		= true
				});

			
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletsBuffer),			mesh.NumMeshlets			 * sizeof(DirectX::Meshlet));
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletsVerticesBuffer),	mesh.MeshletVertices.size()  * sizeof(uint8));
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletsTrianglesBuffer), mesh.MeshletTriangles.size() * sizeof(DirectX::MeshletTriangle));

			//D3D12UploadContext::Upload();
		}

	}
} // namespace Luden

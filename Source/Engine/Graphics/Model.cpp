#include "D3D12/D3D12Device.hpp"
#include "D3D12/D3D12UploadContext.hpp"
#include "Model.hpp"

namespace Luden
{
	void Model::Create(D3D12Device* pDevice)
	{
		ConstantBuffer = pDevice->CreateConstantBuffer(&cbObjectTransforms, sizeof(cbObjectTransforms));

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

			mesh.MeshletsBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Structured,
				.Data			= mesh.Meshlets.data(),
				.NumElements	= mesh.NumMeshlets,
				.Stride			= sizeof(mesh.Meshlets.at(0)),
				.bBindless		= true
				});

			mesh.MeshletVerticesBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Structured,
				.Data			= mesh.MeshletVertices.data(),
				.NumElements	= (uint32)mesh.MeshletVertices.size(),
				.Stride			= sizeof(mesh.MeshletVertices.at(0)),
				.bBindless		= false
				});

			//mesh.MeshletsTrianglesBuffer = pDevice->CreateBuffer({
			//	.BufferUsage	= BufferUsageFlag::Structured,
			//	.Data			= mesh.MeshletTriangles.data(),
			//	.NumElements	= (uint32)mesh.MeshletTriangles.size(),
			//	.Stride			= sizeof(mesh.MeshletTriangles.at(0)),
			//	.bBindless		= false
			//	});

			std::vector<uint32_t> meshletPrimitives;
			for (auto& val : mesh.MeshletTriangles)
			{
				meshletPrimitives.push_back(static_cast<uint32_t>(val));
			}

			//auto triangles = reinterpret_cast<uint32*>(mesh.MeshletTriangles.data());
			//size_t trianglesCount = mesh.MeshletTriangles.size() / sizeof(uint32);

			mesh.MeshletTrianglesBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Structured,
				.Data			= meshletPrimitives.data(),
				.NumElements	= (uint32)meshletPrimitives.size(),
				.Stride			= sizeof(uint32),
				.bBindless		= false
				});
			//.Stride = sizeof(mesh.MeshletTriangles.at(0)),

			mesh.MeshletBoundsBuffer = pDevice->CreateBuffer({
				.BufferUsage	= BufferUsageFlag::Structured,
				.Data			= mesh.MeshletBounds.data(),
				.NumElements	= (uint32)mesh.MeshletBounds.size(),
				.Stride			= sizeof(mesh.MeshletBounds.at(0)),
				.bBindless		= false
				});


			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.VertexBuffer),			(uint64)mesh.NumVertices	 * sizeof(Vertex));
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.IndexBuffer),				(uint64)mesh.NumIndices		 * sizeof(uint32));
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletsBuffer),			(uint64)mesh.NumMeshlets	 * sizeof(mesh.Meshlets.at(0)));
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletVerticesBuffer),	mesh.MeshletVertices.size()  * sizeof(mesh.MeshletVertices.at(0)));
			//D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletTrianglesBuffer), mesh.MeshletTriangles.size() * sizeof(mesh.MeshletTriangles.at(0)));
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletTrianglesBuffer),	meshletPrimitives.size()	 * sizeof(uint32));
			D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletBoundsBuffer),		mesh.MeshletBounds.size()	 * sizeof(mesh.MeshletBounds.at(0)));

			//D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletsBuffer), (uint64)mesh.NumMeshlets * sizeof(DirectX::Meshlet));
			//D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletsVerticesBuffer), mesh.MeshletVertices.size() * sizeof(uint8));
			//D3D12UploadContext::UploadBuffer(pDevice->Buffers.at(mesh.MeshletsTrianglesBuffer), mesh.MeshletTriangles.size() * sizeof(DirectX::MeshletTriangle));

			D3D12UploadContext::Upload();
		}

	}
} // namespace Luden

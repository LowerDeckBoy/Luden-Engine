#include "Config.hpp"
#include "D3D12/D3D12UploadContext.hpp"
#include "Model.hpp"
#include <Core/Defines.hpp>
#include <Core/Logger.hpp>

namespace Luden
{
	Model::~Model()
	{
	
	}

	void Model::Create(D3D12Device* pDevice)
	{
		m_ParentDevice = pDevice;

		CreateResources();

		D3D12UploadContext::Upload();

	}

	void Model::Release()
	{
		for (auto texture : Textures)
		{
			delete texture;
		}

		//Entity::RemoveComponent<ecs::TransformComponent>();
		//Entity::RemoveComponent<ecs::NameComponent>();

		//Entity::Destroy();
	}

	void Model::CreateResources()
	{
		ConstantBuffer = m_ParentDevice->CreateConstantBuffer(&cbObjectTransforms, sizeof(cbObjectTransforms));

		for (auto& mesh : Meshes)
		{
			mesh.VertexBuffer = m_ParentDevice->CreateBuffer({
				.BufferUsage = BufferUsageFlag::Structured,
				.Data = mesh.Vertices.data(),
				.NumElements = mesh.NumVertices,
				.Stride = sizeof(Vertex),
				.bBindless = true,
				.Name = mesh.Name + " Vertex Buffer"
				});

			mesh.IndexBuffer = m_ParentDevice->CreateBuffer({
				.BufferUsage = BufferUsageFlag::Index,
				.Data = mesh.Indices.data(),
				.NumElements = mesh.NumIndices,
				.Stride = sizeof(uint32),
				.bBindless = false,
				.Name = mesh.Name + " Index Buffer"
				});

			mesh.IndexBufferView = D3D12_INDEX_BUFFER_VIEW{
				.BufferLocation = m_ParentDevice->Buffers.at(mesh.IndexBuffer)->GetHandleRaw()->GetGPUVirtualAddress(),
				.SizeInBytes = mesh.NumIndices * (uint32)sizeof(uint32),
				.Format = DXGI_FORMAT_R32_UINT
			};

			mesh.MeshletsBuffer = m_ParentDevice->CreateBuffer({
				.BufferUsage = BufferUsageFlag::Structured,
				.Data = mesh.Meshlets.data(),
				.NumElements = mesh.NumMeshlets,
				.Stride = sizeof(mesh.Meshlets.at(0)),
				.bBindless = true,
				.Name = mesh.Name + " Meshlet Buffer"
				});

			mesh.MeshletVerticesBuffer = m_ParentDevice->CreateBuffer({
				.BufferUsage = BufferUsageFlag::Structured,
				.Data = mesh.MeshletVertices.data(),
				.NumElements = (uint32)mesh.MeshletVertices.size(),
				.Stride = sizeof(mesh.MeshletVertices.at(0)),
				.bBindless = true,
				.Name = mesh.Name + " MeshletVertices Buffer"
				});

			// Repacking Triangles
			std::vector<uint32> meshletTrianglesRepacked;
			for (auto& meshlet : mesh.Meshlets)
			{
				// Save triangle offset for current meshlet
				uint32 triangleOffset = static_cast<uint32>(meshletTrianglesRepacked.size());

				// Repack to uint32
				for (uint32 i = 0; i < meshlet.triangle_count; ++i)
				{
					uint32 i0 = 3 * i + 0 + meshlet.triangle_offset;
					uint32 i1 = 3 * i + 1 + meshlet.triangle_offset;
					uint32 i2 = 3 * i + 2 + meshlet.triangle_offset;

					uint8_t  vIdx0 = mesh.MeshletTriangles[i0];
					uint8_t  vIdx1 = mesh.MeshletTriangles[i1];
					uint8_t  vIdx2 = mesh.MeshletTriangles[i2];
					uint32_t packed =
						((static_cast<uint32>(vIdx0) & 0xFF) << 0) |
						((static_cast<uint32>(vIdx1) & 0xFF) << 8) |
						((static_cast<uint32>(vIdx2) & 0xFF) << 16);
					meshletTrianglesRepacked.push_back(packed);
				}

				// Update triangle offset for current meshlet
				meshlet.triangle_offset = triangleOffset;
			}

			mesh.MeshletTrianglesBuffer = m_ParentDevice->CreateBuffer({
				.BufferUsage = BufferUsageFlag::Structured,
				.Data = meshletTrianglesRepacked.data(),
				.NumElements = (uint32)meshletTrianglesRepacked.size(),
				.Stride = sizeof(meshletTrianglesRepacked.at(0)),
				.bBindless = true,
				.Name = mesh.Name + " MeshletTrianglesBuffer"
				});

			mesh.MeshletBoundsBuffer = m_ParentDevice->CreateBuffer({
				.BufferUsage = BufferUsageFlag::Structured,
				.Data = mesh.MeshletBounds.data(),
				.NumElements = (uint32)mesh.MeshletBounds.size(),
				.Stride = sizeof(mesh.MeshletBounds.at(0)),
				.bBindless = true,
				.Name = mesh.Name + " MeshletBounds Buffer"
				});

			D3D12UploadContext::UploadBuffer(m_ParentDevice->Buffers.at(mesh.VertexBuffer),				(uint64)mesh.NumVertices		* sizeof(Vertex));
			D3D12UploadContext::UploadBuffer(m_ParentDevice->Buffers.at(mesh.IndexBuffer),				(uint64)mesh.NumIndices			* sizeof(uint32));
			D3D12UploadContext::UploadBuffer(m_ParentDevice->Buffers.at(mesh.MeshletsBuffer),			(uint64)mesh.NumMeshlets		* sizeof(mesh.Meshlets.at(0)));
			D3D12UploadContext::UploadBuffer(m_ParentDevice->Buffers.at(mesh.MeshletVerticesBuffer),	(uint64)mesh.NumMeshletVertices * sizeof(mesh.MeshletVertices.at(0)));
			D3D12UploadContext::UploadBuffer(m_ParentDevice->Buffers.at(mesh.MeshletTrianglesBuffer),	meshletTrianglesRepacked.size() * sizeof(uint32));
			D3D12UploadContext::UploadBuffer(m_ParentDevice->Buffers.at(mesh.MeshletBoundsBuffer),		mesh.MeshletBounds.size()		* sizeof(mesh.MeshletBounds.at(0)));

		}

	}

} // namespace Luden

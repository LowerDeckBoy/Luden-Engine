#pragma once

#include "ECS/Components/BoundingBoxComponent.hpp"
#include <Core/Types.hpp>
#include <DirectXMath.h>
#include <DirectXMesh.h>
#include <vector>

namespace Luden
{
	constexpr uint32 MeshletMaxTriangles	= 126;
	constexpr uint32 MeshletMaxVertices		= 64;

	struct Material;

	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TexCoord;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT4 Tangent;
	};

	struct StaticMesh
	{
		std::vector<uint32>	Indices;

		std::vector<Vertex>	Vertices;

		std::vector<DirectX::Meshlet> Meshlets;
		std::vector<uint8> MeshletVertices;
		std::vector<DirectX::MeshletTriangle> MeshletTriangles;
		
		ecs::BoundingBoxComponent BoundingBox;

		DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();

		uint32 VertexBuffer;
		uint32 IndexBuffer;

		D3D12_INDEX_BUFFER_VIEW IndexBufferView;

		uint32 MeshletsBuffer;
		uint32 MeshletsVerticesBuffer;
		uint32 MeshletsTrianglesBuffer;

		uint32 NumVertices = 0;
		uint32 NumIndices  = 0;

		uint32 NumMeshlets = 0;

	};

} // namespace Luden

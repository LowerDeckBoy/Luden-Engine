#pragma once

#include "ECS/Components/BoundingBoxComponent.hpp"
#include "ECS/Components/TransformComponent.hpp"
#include "Material.hpp"
#include <Core/Math/Math.hpp>
#include <DirectXMath.h>
#include <DirectXMesh.h>
#include <meshoptimizer/meshoptimizer.h>
#include <vector>

namespace Luden
{
	constexpr uint32 MeshletMaxTriangles	= 124;
	constexpr uint32 MeshletMaxVertices		= 64;

	struct Material;

	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TexCoord;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT3 Bitangent;
	};

	// TODO:
	struct VertexStream
	{
		std::vector<DirectX::XMFLOAT3> Positions;
		std::vector<DirectX::XMFLOAT2> TexCoords;
		std::vector<DirectX::XMFLOAT3> Normals;
		std::vector<DirectX::XMFLOAT3> Tangents;
	};

	struct FRaytracingInstanceDesc
	{
		DirectX::XMFLOAT3X4 Transform;
		uint32 InstanceID : 24;
		uint32 InstanceMask : 8;
		uint32 InstanceContributionToHitGroupIndex : 24;
		uint32 Flags : 8;
		uint64 AccelerationStructure; // GPU Address
	};

	struct FMeshletBounds
	{
		DirectX::XMFLOAT3	Center;
		f32					Radius;

		DirectX::XMFLOAT3	ConeApex[3];
		DirectX::XMFLOAT3	ConeAxis[3];
		f32					ConeCutoff; /* = cos(angle/2) */
	};


	struct StaticMesh
	{
		std::string Name;

		uint32 MaterialId = 0xffffffff;

		std::vector<uint32>	Indices;
		std::vector<Vertex>	Vertices;

		std::vector<meshopt_Meshlet>	Meshlets;
		std::vector<uint32>				MeshletVertices;
		std::vector<uint8>				MeshletTriangles;
		std::vector<FMeshletBounds>		MeshletBounds;

		ecs::BoundingBoxComponent		BoundingBox;
		ecs::TransformComponent			Transform;

		EAlphaMode AlphaMode;

		uint32 VertexBuffer;
		uint32 IndexBuffer;

		D3D12_INDEX_BUFFER_VIEW IndexBufferView;

		// Used for TLAS building.
		FRaytracingInstanceDesc RaytracingInstanceDesc{};

		uint32 MeshletsBuffer;
		uint32 MeshletVerticesBuffer;
		uint32 MeshletTrianglesBuffer;
		uint32 MeshletBoundsBuffer;

		uint32 BLASBuffer;

		uint32 NumVertices	= 0;
		uint32 NumIndices	= 0;
		uint32 NumMeshlets	= 0;
		uint32 NumMeshletVertices  = 0;
		uint32 NumMeshletTriangles = 0;

	};

} // namespace Luden

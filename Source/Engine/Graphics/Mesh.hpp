#pragma once

#include "ECS/Components/BoundingBoxComponent.hpp"
//#include <Core/Types.hpp>
#include "Material.hpp"
#include <DirectXMath.h>
#include <DirectXMesh.h>
#include <vector>
#include <unordered_map>
#include <meshoptimizer/meshoptimizer.h>

namespace Luden
{
	constexpr uint32 MeshletMaxTriangles = 124;
	constexpr uint32 MeshletMaxVertices = 64;

	struct Material;

	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TexCoord;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT4 Tangent;
	};

	struct VertexStream
	{
		std::vector<DirectX::XMFLOAT3> Positions;
		std::vector<DirectX::XMFLOAT2> TexCoords;
		std::vector<DirectX::XMFLOAT3> Normals;
		std::vector<DirectX::XMFLOAT4> Tangents;
	};

	struct FRaytracingInstance
	{

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
		uint32 MaterialIndex = 0xFFFFFFFF;

		std::vector<uint32>	Indices;

		std::vector<Vertex>	Vertices;

		//std::vector<DirectX::Meshlet>			Meshlets;
		//std::vector<uint8>						MeshletVertices;
		//std::vector<DirectX::MeshletTriangle>	MeshletTriangles;
		//std::vector<DirectX::CullData>			MeshletCull;

		std::vector<meshopt_Meshlet>	Meshlets;
		std::vector<uint32>				MeshletVertices;
		std::vector<uint8>				MeshletTriangles;
		std::vector<FMeshletBounds>		MeshletBounds;
		//std::vector<meshopt_Bounds>		MeshletBounds;

		ecs::BoundingBoxComponent BoundingBox;

		DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();

		uint32 VertexBuffer;
		uint32 IndexBuffer;

		D3D12_INDEX_BUFFER_VIEW IndexBufferView;

		uint32 MeshletsBuffer;
		uint32 MeshletVerticesBuffer;
		uint32 MeshletTrianglesBuffer;
		uint32 MeshletBoundsBuffer;

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		uint32 NumMeshlets = 0;

		// TEST
		VertexStream VertexStream;
		uint32 PositionOffset	= 0;
		uint32 TexCoordOffset	= 0;
		uint32 NormalOffset		= 0;
		uint32 TangentOffset	= 0;

	};

} // namespace Luden

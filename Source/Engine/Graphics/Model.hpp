#pragma once

#include "D3D12/D3D12Buffer.hpp"

#include "ECS/Components/NameComponent.hpp"
#include "ECS/Components/TransformComponent.hpp"

#include "ECS/Entity.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "RHI/Constants.hpp"
#include <Core/File.hpp>

namespace Luden
{
	class Model : public Entity
	{
	public:
		Model() = default;
		
		// Create Buffer Resources for this model.
		void Create(D3D12Device* pDevice);

		// Single StaticMesh built from submeshes?
		// Single Material built from submaterials?

		std::vector<StaticMesh> Meshes;
		std::vector<Material>	Materials;
		//std::unordered_map<Filepath, Material>	Materials;

		uint32 ConstantBuffer;
		Constants::ObjectTranforms cbObjectTransforms{};

		Filepath GetFilepath()
		{
			return m_Filepath;
		}

		void SetFilepath(Filepath Path)
		{
			m_Filepath = Path;
		}

		//uint32 TotalVertexCount;
		//uint32 TotalIndexCount;
		//uint32 TotalMeshletCount;

	private:
		Filepath m_Filepath;

	};
} // namespace Luden

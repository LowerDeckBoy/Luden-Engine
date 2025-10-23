#pragma once

#include "D3D12/D3D12Buffer.hpp"
#include "D3D12/D3D12Texture.hpp"

#include "ECS/Components/NameComponent.hpp"
#include "ECS/Components/TransformComponent.hpp"

#include "ECS/Entity.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "RHI/Constants.hpp"
#include <Core/File.hpp>

namespace Luden
{
	class Scene;

	class Model : public Entity
	{
	public:
		Model() = default;
		~Model();
		
		// Create and upload Buffer Resources.
		void Create(D3D12Device* pDevice);

		void Release();

		std::vector<StaticMesh>		Meshes;
		std::vector<StaticMesh>		OpaqueMeshes;
		std::vector<StaticMesh>		BlendMeshes;

		std::vector<D3D12Texture*>	Textures;

		uint32 ConstantBuffer;
		uint32 TransformID;

		// TODO:
		//void SetMaterial();

		INLINE Filepath GetFilepath()
		{
			return m_Filepath;
		}

		INLINE void SetFilepath(Filepath Path)
		{
			m_Filepath = Path;
		}

		Scene* m_ParentScene = nullptr;
	private:
		Filepath m_Filepath;

		// Createb buffers related resources for this model.
		void CreateResources();


		D3D12Device* m_ParentDevice = nullptr;


	};
} // namespace Luden

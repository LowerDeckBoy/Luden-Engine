#pragma once

#include <Core/File.hpp>
#include <Core/Types.hpp>
#include <DirectXMath.h>
#include <assimp/Importer.hpp>
#include <assimp/GltfMaterial.h>
#include <map>

struct FNode;
struct aiMesh;
struct aiNode;
struct cgltf_scene;
struct cgltf_data;
struct cgltf_node;

//
// TODO:
// Need to resolve scaling issue.
//

namespace Luden
{
	class D3D12Device;
	class D3D12Texture;
	class Model;

	struct StaticMesh;
	struct Material;
	struct TextureDesc;

	struct FAssimpLoadingData
	{
		const aiScene* Scene;
		Filepath Path;

		std::vector<StaticMesh> Meshes;
		std::vector<Material>	Materials;
		
		// Test
		// Not using maps should work just as fine.
		std::vector<aiMaterial*>	UniqueAssimpMaterials;
		std::vector<Material>		UniqueMaterials;
		std::vector<D3D12Texture*>	ModelTextures;

		// Currently not using any animation, thus nodes aren't needed at all.
		//std::vector<FNode*>	Nodes;
	};

	class AssetImporter
	{
	public:

		bool ImportStaticMesh(Filepath Path, Model& OutModel);

		bool ImportAssimpModel(Filepath Path, Model& OutModel);

		void LoadTexture(Filepath Path, TextureDesc& OutDesc);

		void LoadTexture2D(Filepath Path, TextureDesc& OutDesc);
		void LoadTextureDDS(Filepath Path);

		D3D12Device* Device;

	private:
		// Load all materials from Assimp model and map them.
		void LoadMaterials(FAssimpLoadingData& SceneData);

		void LoadMaterial(FAssimpLoadingData& SceneData, const aiMesh* pMesh, StaticMesh& Mesh);

		void TraverseNode(FAssimpLoadingData& SceneData, aiNode* pNode);

		// TODO:
		// Requires rebuild
		//void TraverseNode(const cgltf_data* pScene, const cgltf_node* pNode, Model& OutModel, FNode* pParentNode, DirectX::XMMATRIX ParentMatrix = DirectX::XMMatrixIdentity());

		// Use meshoptimizer to optimize mesh data and generate meshlets.
		void BuildMesh(StaticMesh& Mesh);

		void CreateTexture(D3D12Texture* pTexture);

		

	};
} // namespace Luden

#pragma once

#include <Core/File.hpp>
#include <Core/Types.hpp>
#include <DirectXMath.h>
#include <assimp/Importer.hpp>

struct FNode;
struct aiMesh;
struct aiNode;
struct cgltf_scene;
struct cgltf_data;
struct cgltf_node;

namespace Luden
{
	class Model;
	struct StaticMesh;
	struct Material;

	struct FAssimpLoadingData
	{
		const aiScene* Scene;

		std::vector<StaticMesh> Meshes;
		std::vector<Material>	Materials;

		// Currently not using any animation, thus nodes aren't needed at all.
		//std::vector<FNode*>	Nodes;
	};

	class AssetImporter
	{
	public:

		bool ImportStaticMesh(Filepath Path, Model& OutModel);

		bool ImportAssimpModel(Filepath Path, Model& OutModel);

		void LoadTexture(Filepath Path);
		// .jpg, .jpeg, .png
		void LoadTexture2D(Filepath Path);
		void LoadTextureDDS(Filepath Path);

	private:
		void LoadMaterials(FAssimpLoadingData& SceneData);

		void TraverseNode(FAssimpLoadingData& SceneData, aiNode* pNode);

		// TODO:
		// Requires rebuild
		void TraverseNode(const cgltf_data* pScene, const cgltf_node* pNode, Model& OutModel, FNode* pParentNode, DirectX::XMMATRIX ParentMatrix = DirectX::XMMatrixIdentity());

		// Use meshoptimizer to optimize mesh data and generate meshlets.
		void BuildMesh(StaticMesh& Mesh);

	};
} // namespace Luden

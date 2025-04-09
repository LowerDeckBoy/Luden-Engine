#pragma once

#include <Core/File.hpp>
#include <Core/Types.hpp>
#include <DirectXMath.h>
#include <assimp/Importer.hpp>

struct FNode;
struct aiMesh;
struct aiNode;
struct aiMaterial;

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
		
		std::vector<aiMaterial*>	UniqueAssimpMaterials;
		std::vector<Material>		UniqueMaterials;
		std::vector<D3D12Texture*>	ModelTextures;

		// Currently not using any animation, thus nodes aren't needed at all.
		//std::vector<FNode*>	Nodes;
	};

	class AssetImporter
	{
	public:
		AssetImporter() = default;
		~AssetImporter() = default;

		bool ImportStaticMesh(Filepath Path, Model& OutModel);

		bool ImportAssimpModel(Filepath Path, Model& OutModel);

		D3D12Texture* LoadTexture(Filepath Path);

		void LoadTexture2D(Filepath Path, D3D12Texture* pTexture);
		void LoadTextureDDS(Filepath Path, D3D12Texture* pTexture);

		D3D12Device* Device;

	private:
		// Load all materials from Assimp model.
		void LoadMaterials(FAssimpLoadingData& SceneData);

		// Process Assimp node recursively.
		// Get information about it's meshes, matrix transformation and material.
		void TraverseNode(FAssimpLoadingData& SceneData, aiNode* pNode);

		// Use meshoptimizer to optimize mesh data and generate meshlets.
		void BuildMesh(StaticMesh& Mesh);

	};
} // namespace Luden

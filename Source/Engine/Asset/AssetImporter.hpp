#pragma once

#include "Graphics/Model.hpp"
#include <Core/File.hpp>
#include <Core/Logger.hpp>
#include <Core/Types.hpp>



struct FNode;

struct aiScene;
struct aiMesh;
struct aiNode;
struct aiMaterial;

namespace fastgltf 
{ 
	class Asset;
	class GltfDataBuffer; 
	struct Node;

	namespace math
	{
		template <typename T, std::size_t N, std::size_t M>
		class mat;

		//using fmat<4, 4> = mat<float, 4, 4>;
		//using fmat4x4 = fmat<4, 4>;
		//using fmat4x4 = mat<float, 4, 4>;
	}
}

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
		
		//std::vector<aiMaterial*>	UniqueAssimpMaterials;
		std::vector<Material>		UniqueMaterials;
		std::vector<D3D12Texture*>	ModelTextures;

		// Temp
		std::vector<std::string> LoadedPaths;

		// Currently not using any animation, thus nodes aren't needed at all.
		//std::vector<FNode*>	Nodes;
	};



	//
	// TODO:
	// - Add temporal cache for already loaded textures to avoid duplicates.
	// - Add fastgltf based loader for glTF 2.0 assets and keep Assimp for fbx/obj.
	// - Clean-up material loading.
	class AssetImporter
	{
	public:
		AssetImporter() = default;
		~AssetImporter() = default;
		
		bool ImportStaticMesh(Filepath Path, Model& OutModel);

		D3D12Texture* LoadTexture(Filepath Path);

		void LoadTexture2D(Filepath Path, D3D12Texture* pTexture);
		void LoadTextureDDS(Filepath Path, D3D12Texture* pTexture);

		D3D12Device* Device;

	private:
		// Use meshoptimizer to optimize mesh data and generate meshlets.
		void BuildMesh(StaticMesh& Mesh);

		bool ImportAssimpModel(Filepath Path, Model& OutModel);

		bool ImportFastglftModel(Filepath Path, Model& OutModel);

		// Load all materials from Assimp model.
		void LoadMaterials(FAssimpLoadingData& SceneData);

		// Process Assimp node recursively.
		// Get information about it's meshes, matrix transformation and material.
		void TraverseNode(FAssimpLoadingData& SceneData, aiNode* pNode);
		
		bool IsTextureLoaded(FAssimpLoadingData& SceneData, Filepath Path);

	};
} // namespace Luden

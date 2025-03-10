#pragma once

#include <Core/File.hpp>
#include <assimp/Importer.hpp>

struct aiMesh;
struct cgltf_scene;
struct cgltf_data;

namespace Luden
{
	class Model;
	struct StaticMesh;

	class AssetImporter
	{
	public:

		void ImportStaticMesh(Filepath Path, Model& OutModel);
		void ImportStaticMesh(Filepath Path, Model& OutModel, bool bCGLTF);
		//void ImportStaticMesh(Filepath Path, Model& OutModel);

		void LoadTexture(Filepath Path);
		void LoadTexture2D(Filepath Path);
		void LoadTextureDDS(Filepath Path);

	private:
		void LoadStaticMesh(const aiScene* pScene, Model& OutModel);
		void LoadMaterials(const aiScene* pScene, Model& OutModel);

		void LoadStreamedStaticMesh(const aiScene* pScene, Model& OutModel, bool test);
		void LoadStaticMesh(const cgltf_data* pScene, Model& OutModel);

		// Use meshoptimizer to optimize mesh data and generate meshlets.
		void BuildMesh(StaticMesh& Mesh);

	};
} // namespace Luden

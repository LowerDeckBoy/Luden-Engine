#pragma once

#include <Core/File.hpp>

#include <assimp/Importer.hpp>

#include "Graphics/Model.hpp"

namespace Luden
{
	class AssetImporter
	{
	public:

		void ImportStaticMesh(Filepath Path, Model& OutModel, bool bGenerateMeshlets = false);

		//void LoadTexture();

	private:
		void LoadStaticMesh(const aiScene* pScene, Model& OutModel);

		void BuildMeshlets(StaticMesh& Mesh);
		
	};
} // namespace Luden

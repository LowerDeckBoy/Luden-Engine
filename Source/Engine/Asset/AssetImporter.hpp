#pragma once

#include <Core/File.hpp>
#include <assimp/Importer.hpp>

namespace Luden
{
	class Model;

	class AssetImporter
	{
	public:

		void ImportStaticMesh(Filepath Path, Model& OutModel);

		//void LoadTexture();

	private:
		void LoadStaticMesh(const aiScene* pScene, Model& OutModel);

	};
} // namespace Luden

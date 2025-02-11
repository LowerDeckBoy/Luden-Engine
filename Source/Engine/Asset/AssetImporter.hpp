#pragma once

#include <Core/File.hpp>

#include <assimp/Importer.hpp>

#include "Graphics/Mesh.hpp"

namespace Luden
{
	class AssetImporter
	{
	public:

		bool Import(Filepath Path, Mesh& OutMesh);

		void ImportStaticMesh(Filepath Path, StaticMesh& OutMesh, bool bGenerateMeshlets = false);

		void LoadTexture();

	private:
		void LoadStaticMesh(const aiScene* pScene, StaticMesh& OutMesh);

	};
} // namespace Luden

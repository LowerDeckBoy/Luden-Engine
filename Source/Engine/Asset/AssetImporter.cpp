#include "AssetImporter.hpp"
#include <Core/Logger.hpp>

#include <assimp/GltfMaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Luden
{
	void AssetImporter::ImportStaticMesh(Filepath Path, StaticMesh& OutMesh, bool /* bGenerateMeshlets */)
	{
		if (!File::Exists(Path))
		{
			LOG_WARNING("Invalid path");

			return;
		}

		constexpr int32_t loadFlags =
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes |
			aiProcess_PreTransformVertices |
			aiProcess_ImproveCacheLocality;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Path.string(), loadFlags);

		if (!scene || !scene->mRootNode || !scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			LOG_WARNING("Failed to load model mesh");

			return;
		}

		OutMesh.Submeshes.reserve(scene->mNumMeshes);

		LoadStaticMesh(scene, OutMesh);

	}

	void AssetImporter::LoadStaticMesh(const aiScene* pScene, StaticMesh& OutMesh)
	{
		if (!pScene->HasMeshes())
		{
			LOG_WARNING("Model doesn't contain any meshes.");

			return;
		}

		for (uint32_t i = 0; i < pScene->mNumMeshes; ++i)
		{
			const auto& mesh = pScene->mMeshes[i];

			Subset submesh{};
			submesh.BaseVertex = static_cast<uint32_t>(OutMesh.Positions.size());
			submesh.BaseIndex = static_cast<uint32_t>(OutMesh.Indices.size());

			submesh.NumVertices = mesh->mNumVertices;

			submesh.NumIndices = 3 * mesh->mNumFaces;

			for (uint32_t vertexId = 0; vertexId < mesh->mNumVertices; ++vertexId)
			{
				if (mesh->HasPositions())
				{
					OutMesh.Positions.emplace_back(*(DirectX::XMFLOAT3*)(&mesh->mVertices[vertexId]));
				}
				else
				{
					OutMesh.Positions.emplace_back(DirectX::XMFLOAT3());
				}

				if (mesh->HasTextureCoords(0))
				{
					OutMesh.TexCoords.emplace_back(*(DirectX::XMFLOAT2*)(&mesh->mTextureCoords[0][vertexId]));
				}
				else
				{
					OutMesh.TexCoords.emplace_back(DirectX::XMFLOAT2());
				}

				if (mesh->HasNormals())
				{
					OutMesh.Normals.emplace_back(*(DirectX::XMFLOAT3*)(&mesh->mNormals[vertexId]));
				}
				else
				{
					OutMesh.Normals.emplace_back(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
				}

				if (mesh->HasTangentsAndBitangents())
				{
					OutMesh.Tangents.emplace_back(*(DirectX::XMFLOAT3*)(&mesh->mTangents[vertexId]));
					//OutMesh.Bitangents.emplace_back(*(DirectX::XMFLOAT3*)(&mesh->mBitangents[vertexId]));
				}
				else
				{
					OutMesh.Tangents.emplace_back(DirectX::XMFLOAT3());
					//OutMesh.Bitangents.emplace_back(DirectX::XMFLOAT3());
				}
			}

			if (mesh->HasFaces())
			{

				for (uint32_t faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx)
				{
					aiFace& face = mesh->mFaces[faceIdx];

					for (uint32_t idx = 0; idx < face.mNumIndices; ++idx)
					{
						OutMesh.Indices.push_back(face.mIndices[idx]);
					}
				}
			}

			OutMesh.Submeshes.push_back(submesh);
		}
	}
} // namespace Luden

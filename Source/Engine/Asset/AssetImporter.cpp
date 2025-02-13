#include "AssetImporter.hpp"
#include <Core/Logger.hpp>

#include <assimp/GltfMaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Luden
{
	void AssetImporter::ImportStaticMesh(Filepath Path, Model& OutModel, bool bGenerateMeshlets)
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
			importer.FreeScene();

			return;
		}

		OutModel.Meshes.reserve(scene->mNumMeshes);

		LoadStaticMesh(scene, OutModel);

		if (bGenerateMeshlets)
		{
			for (auto& mesh : OutModel.Meshes)
			{
				BuildMeshlets(mesh);
			}
		}

		importer.FreeScene();

	}

	void AssetImporter::LoadStaticMesh(const aiScene* pScene, Model& OutModel)
	{
		if (!pScene->HasMeshes())
		{
			LOG_WARNING("Model doesn't contain any meshes.");

			return;
		}

		for (uint32_t i = 0; i < pScene->mNumMeshes; ++i)
		{
			const auto& mesh = pScene->mMeshes[i];

			StaticMesh meshData{};

			for (uint32_t vertexId = 0; vertexId < mesh->mNumVertices; ++vertexId)
			{
				if (mesh->HasPositions())
				{
					meshData.Positions.emplace_back(*(DirectX::XMFLOAT3*)(&mesh->mVertices[vertexId]));
				}
				else
				{
					meshData.Positions.emplace_back(DirectX::XMFLOAT3());
				}

				if (mesh->HasTextureCoords(0))
				{
					meshData.TexCoords.emplace_back(*(DirectX::XMFLOAT2*)(&mesh->mTextureCoords[0][vertexId]));
				}
				else
				{
					meshData.TexCoords.emplace_back(DirectX::XMFLOAT2());
				}

				if (mesh->HasNormals())
				{
					meshData.Normals.emplace_back(*(DirectX::XMFLOAT3*)(&mesh->mNormals[vertexId]));
				}
				else
				{
					meshData.Normals.emplace_back(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
				}

				if (mesh->HasTangentsAndBitangents())
				{
					meshData.Tangents.emplace_back(*(DirectX::XMFLOAT4*)(&mesh->mTangents[vertexId]));
				}
				else
				{
					meshData.Tangents.emplace_back(DirectX::XMFLOAT4());
				}
			}

			if (mesh->HasFaces())
			{

				for (uint32_t faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx)
				{
					aiFace& face = mesh->mFaces[faceIdx];

					for (uint32_t idx = 0; idx < face.mNumIndices; ++idx)
					{
						meshData.Indices.push_back(face.mIndices[idx]);
					}
				}
			}

			OutModel.Meshes.push_back(meshData);
		}
	}

	void AssetImporter::BuildMeshlets(StaticMesh& Mesh)
	{
		DirectX::ComputeMeshlets(
			Mesh.Indices.data(), Mesh.Indices.size() / 3,
			Mesh.Positions.data(), Mesh.Positions.size(),
			nullptr,
			Mesh.Meshlets, Mesh.MeshletVertices,
			Mesh.MeshletTriangles,
			MeshletMaxTriangles, MeshletMaxVertices
		);
	}
} // namespace Luden

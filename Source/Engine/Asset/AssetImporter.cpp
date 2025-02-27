#include "Graphics/Model.hpp"
#include "AssetImporter.hpp"
#include <Core/Logger.hpp>

#include <assimp/GltfMaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Luden
{
	void AssetImporter::ImportStaticMesh(Filepath Path, Model& OutModel)
	{
		const std::string& path = Path.string();
		if (!File::Exists(Path))
		{
			LOG_WARNING("{0} path is invalid.", path);

			return;
		}

		constexpr int32 loadFlags =
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes |
			aiProcess_PreTransformVertices |
			aiProcess_ImproveCacheLocality |
			aiProcess_GenBoundingBoxes;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Path.string(), (uint32)loadFlags);

		if (!scene || !scene->mRootNode || !scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			LOG_WARNING("Failed to load model: {}", path);
			importer.FreeScene();

			return;
		}

		OutModel.Meshes.reserve(scene->mNumMeshes);

		LoadStaticMesh(scene, OutModel);

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

			meshData.BoundingBox.Min = *(DirectX::XMFLOAT3*)(&mesh->mAABB.mMin);
			meshData.BoundingBox.Max = *(DirectX::XMFLOAT3*)(&mesh->mAABB.mMax);
			std::vector<DirectX::XMFLOAT3> positions;

			meshData.Vertices.reserve(mesh->mNumVertices);

			for (uint32_t vertexId = 0; vertexId < mesh->mNumVertices; ++vertexId)
			{
				Vertex vertex{};

				if (mesh->HasPositions())
				{
					vertex.Position = *(DirectX::XMFLOAT3*)(&mesh->mVertices[vertexId]);
					positions.push_back(vertex.Position);
				}

				if (mesh->HasTextureCoords(0))
				{
					vertex.TexCoord = *(DirectX::XMFLOAT2*)(&mesh->mTextureCoords[0][vertexId]);
				}

				if (mesh->HasNormals())
				{
					vertex.Normal = *(DirectX::XMFLOAT3*)(&mesh->mNormals[vertexId]);
				}

				if (mesh->HasTangentsAndBitangents())
				{
					vertex.Tangent = *(DirectX::XMFLOAT4*)(&mesh->mTangents[vertexId]);
				}

				meshData.Vertices.push_back(vertex);
			}

			if (mesh->HasFaces())
			{
				meshData.Indices.reserve(static_cast<usize>(mesh->mNumFaces * 3));

				for (uint32_t faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx)
				{
					aiFace& face = mesh->mFaces[faceIdx];

					for (uint32_t idx = 0; idx < face.mNumIndices; ++idx)
					{
						meshData.Indices.push_back(face.mIndices[idx]);
					}
				}
			}

			DirectX::ComputeMeshlets(
				meshData.Indices.data(), meshData.Indices.size() / 3,
				positions.data(), positions.size(),
				nullptr,
				meshData.Meshlets, meshData.MeshletVertices,
				meshData.MeshletTriangles,
				MeshletMaxVertices, MeshletMaxTriangles);

			meshData.NumVertices	= static_cast<uint32>(meshData.Vertices.size());
			meshData.NumIndices		= static_cast<uint32>(meshData.Indices.size());
			meshData.NumMeshlets	= static_cast<uint32>(meshData.Meshlets.size());

			OutModel.Meshes.push_back(meshData);
		}
	}
} // namespace Luden

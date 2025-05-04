#include "AssetImporter.hpp"
#include "D3D12/D3D12Texture.hpp"
#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <map>

namespace Luden
{
	// For custom assimp related functionality.
	namespace assimp
	{
		struct FAssimpLoadingData
		{
			const aiScene* Scene;
			Filepath Path;

			std::vector<StaticMesh>		Meshes;

			std::vector<Material>		UniqueMaterials;
			std::vector<D3D12Texture*>	ModelTextures;

			std::vector<std::string>	LoadedPaths;
			std::vector<std::pair<uint32, std::pair<Filepath, ETextureType>>> TexturesToLoad;
		};
		

	} // namespace assimp

	static bool IsTextureLoaded(assimp::FAssimpLoadingData& SceneData, Filepath Path);

	static void LoadMaterials(assimp::FAssimpLoadingData& SceneData);

	// Process Assimp node recursively.
	// Get information about it's meshes, matrix transformation and material.
	static void TraverseNode(assimp::FAssimpLoadingData& SceneData, aiNode* pNode);

	bool AssetImporter::ImportAssimpModel(Filepath Path, Model& OutModel)
	{
		constexpr int32 loadFlags =
			aiProcess_ConvertToLeftHanded |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_RemoveRedundantMaterials |
			aiProcess_FindInstances |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_GenBoundingBoxes;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Path.string(), (uint32)loadFlags);

		if (!scene || !scene->mRootNode || !scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			LOG_WARNING("\n\tFailed to load model: {0}, reason: {1}", scene->GetShortFilename(Path.string().c_str()), importer.GetErrorString());

			importer.FreeScene();

			return false;
		}

		OutModel.Meshes.reserve(scene->mNumMeshes);
		OutModel.Materials.reserve(scene->mNumMaterials);

		assimp::FAssimpLoadingData data{};
		data.Scene = scene;
		data.Path = Path;

		LoadMaterials(data);
		TraverseNode(data, scene->mRootNode);

		for (auto& texture : data.TexturesToLoad)
		{
			const auto& materialId  = texture.first;
			const auto& texturePath = texture.second.first;
			const auto& textureType = texture.second.second;

			if (!IsTextureLoaded(data, texturePath))
			{
				D3D12Texture* tex2D = LoadTexture(texturePath);
				
				switch (textureType)
				{
				case ETextureType::BaseColor:
					data.UniqueMaterials.at(materialId).BaseColorIndex = tex2D->ShaderResourceHandle.Index;
					break;
				case ETextureType::Normal:
					data.UniqueMaterials.at(materialId).NormalIndex = tex2D->ShaderResourceHandle.Index;
					break;
				case ETextureType::MetallicRoughness:
					data.UniqueMaterials.at(materialId).MetallicRoughnessIndex = tex2D->ShaderResourceHandle.Index;
					break;
				case ETextureType::Emissive:
					data.UniqueMaterials.at(materialId).EmissiveIndex = tex2D->ShaderResourceHandle.Index;
					break;
				}

				tex2D->SetFilepath(texturePath);
				data.LoadedPaths.push_back(texturePath.string());
				data.ModelTextures.push_back(std::move(tex2D));
			}
			else
			{
				auto textureIndex = FindTextureWithPath(data.ModelTextures, texturePath);

				switch (textureType)
				{
				case ETextureType::BaseColor:
					data.UniqueMaterials.at(materialId).BaseColorIndex = data.ModelTextures.at(textureIndex)->ShaderResourceHandle.Index;
					break;
				case ETextureType::Normal:
					data.UniqueMaterials.at(materialId).NormalIndex = data.ModelTextures.at(textureIndex)->ShaderResourceHandle.Index;
					break;
				case ETextureType::MetallicRoughness:
					data.UniqueMaterials.at(materialId).MetallicRoughnessIndex = data.ModelTextures.at(textureIndex)->ShaderResourceHandle.Index;
					break;
				case ETextureType::Emissive:
					data.UniqueMaterials.at(materialId).EmissiveIndex = data.ModelTextures.at(textureIndex)->ShaderResourceHandle.Index;
					break;
				}
			}
		}

		for (auto& mesh : data.Meshes)
		{
			BuildMesh(mesh);
		}

		OutModel.Meshes		= std::move(data.Meshes);
		OutModel.Materials	= std::move(data.UniqueMaterials);
		OutModel.Textures	= std::move(data.ModelTextures);

		OutModel.SetFilepath(Path);
		importer.FreeScene();

		return true;
	}

	void TraverseNode(assimp::FAssimpLoadingData& SceneData, aiNode* pNode)
	{
		if (!pNode)
		{
			return;
		}
		
		aiMatrix4x4 matrix = pNode->mTransformation;

		DirectX::XMMATRIX transform = *(DirectX::XMMATRIX*)(&matrix);
		aiNode* currNode = pNode;

		while (true)
		{
			if (currNode->mParent)
			{
				currNode = currNode->mParent;
				aiMatrix4x4 pTransform = currNode->mTransformation;
				transform = *(DirectX::XMMATRIX*)(&pTransform) * transform;
			}
			else
			{
				break;
			}
		}

		transform = DirectX::XMMatrixTranspose(transform);

		for (uint32 meshIdx = 0; meshIdx < pNode->mNumMeshes; meshIdx++)
		{
			const auto& mesh = SceneData.Scene->mMeshes[pNode->mMeshes[meshIdx]];
			
			StaticMesh meshData{};
			meshData.Name = mesh->mName.C_Str();
			meshData.Transform.WorldMatrix = transform;

			meshData.BoundingBox.Min = *(DirectX::XMFLOAT3*)(&mesh->mAABB.mMin);
			meshData.BoundingBox.Max = *(DirectX::XMFLOAT3*)(&mesh->mAABB.mMax);

			usize vertexCount = mesh->mNumVertices;

			meshData.Vertices.reserve(vertexCount);

			std::vector<DirectX::XMFLOAT4> positions;
			positions.reserve(vertexCount);
			std::vector<DirectX::XMFLOAT2> texCoords;
			texCoords.reserve(vertexCount);
			std::vector<DirectX::XMFLOAT3> normals;
			normals.reserve(vertexCount);
			std::vector<DirectX::XMFLOAT4> tangents;
			tangents.reserve(vertexCount);
			std::vector<DirectX::XMFLOAT4> bitangents;
			bitangents.reserve(vertexCount);

			for (uint32_t vertexId = 0; vertexId < vertexCount; ++vertexId)
			{
				Vertex vertex{};

				positions.push_back(DirectX::XMFLOAT4(mesh->mVertices[vertexId].x, mesh->mVertices[vertexId].y, mesh->mVertices[vertexId].z, 1.0f));

				if (mesh->HasTextureCoords(0))
				{
					texCoords.push_back(*(DirectX::XMFLOAT2*)(&mesh->mTextureCoords[0][vertexId]));
				}
				else
				{
					texCoords.push_back(DirectX::XMFLOAT2(0.0f, 0.0f));
				}

				if (mesh->HasNormals())
				{
					normals.push_back(*(DirectX::XMFLOAT3*)(&mesh->mNormals[vertexId]));
				}
				else
				{
					normals.push_back(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
				}

				if (mesh->HasTangentsAndBitangents())
				{
					tangents.push_back(DirectX::XMFLOAT4(mesh->mTangents[vertexId].x, mesh->mTangents[vertexId].y, mesh->mTangents[vertexId].z, 1.0f));
					bitangents.push_back(DirectX::XMFLOAT4(mesh->mBitangents[vertexId].x, mesh->mBitangents[vertexId].y, mesh->mBitangents[vertexId].z, 1.0f));
				}
				else
				{
					tangents.push_back(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
					bitangents.push_back(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				}
			}

			DirectX::XMVector4TransformStream(
				positions.data(), sizeof(DirectX::XMFLOAT4),
				positions.data(), sizeof(DirectX::XMFLOAT4),
				vertexCount, meshData.Transform.WorldMatrix);

			DirectX::XMVector3TransformNormalStream(
				normals.data(), sizeof(DirectX::XMFLOAT3),
				normals.data(), sizeof(DirectX::XMFLOAT3),
				vertexCount, meshData.Transform.WorldMatrix);

			DirectX::XMVector4TransformStream(
				tangents.data(), sizeof(DirectX::XMFLOAT4),
				tangents.data(), sizeof(DirectX::XMFLOAT4),
				vertexCount, meshData.Transform.WorldMatrix);

			DirectX::XMVector4TransformStream(
				bitangents.data(), sizeof(DirectX::XMFLOAT4),
				bitangents.data(), sizeof(DirectX::XMFLOAT4),
				vertexCount, meshData.Transform.WorldMatrix);

			//meshopt_remapVertexBuffer(positions.data(), positions.data(), positions.size(), sizeof(Vertex), remap.data());

			for (uint32 vertId = 0; vertId < vertexCount; ++vertId)
			{
				Vertex vout{};

				vout.Position = *(DirectX::XMFLOAT3*)(&positions.at(vertId));
				vout.TexCoord = texCoords.at(vertId);
				vout.Normal = normals.at(vertId);
				vout.Tangent = *(DirectX::XMFLOAT3*)(&tangents.at(vertId));
				vout.Bitangent = *(DirectX::XMFLOAT3*)(&bitangents.at(vertId));

				meshData.Vertices.push_back(vout);
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

			//BuildMesh(meshData);

			DirectX::XMStoreFloat3x4(&meshData.RaytracingInstanceDesc.Transform, meshData.Transform.WorldMatrix);
			meshData.RaytracingInstanceDesc.InstanceID = 0;
			meshData.RaytracingInstanceDesc.InstanceMask = 1;

			// Assign index to unique material to reuse already loaded materials without duplicates.
			meshData.MaterialId = mesh->mMaterialIndex;

			SceneData.Meshes.push_back(meshData);
		}

		if (pNode->mNumChildren > 0)
		{
			for (uint32 childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
			{
				TraverseNode(SceneData, pNode->mChildren[childIdx]);
			}
		}
	}

	void LoadMaterials(assimp::FAssimpLoadingData& SceneData)
	{
		for (uint32 materialIdx = 0; materialIdx < SceneData.Scene->mNumMaterials; ++materialIdx)
		{
			auto* assimpMaterial = SceneData.Scene->mMaterials[materialIdx];

			Material material{};

			aiString path{};
			auto filename = SceneData.Path.filename().stem();

			const std::string pathToParent = std::filesystem::absolute(SceneData.Path).parent_path().string();
			// Some of glTF models use place their textures inside *textures/* directory, but some just don't.
			const std::string pathToTexture = (std::filesystem::exists("textures/") ? "textures/" : "");

			if (assimpMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == aiReturn_SUCCESS)
			{
				Filepath texturePath = std::format("{0}/{1}{2}", pathToParent, pathToTexture, path.C_Str());
				SceneData.TexturesToLoad.push_back({ materialIdx, { texturePath, ETextureType::BaseColor } });
			}

			if (assimpMaterial->GetTexture(aiTextureType_NORMALS, 0, &path) == aiReturn_SUCCESS)
			{
				Filepath texturePath = std::format("{0}/{1}{2}", pathToParent, pathToTexture, path.C_Str());
				SceneData.TexturesToLoad.push_back({ materialIdx, { texturePath, ETextureType::Normal } });
			}

			if (assimpMaterial->GetTexture(aiTextureType_METALNESS, 0, &path) == aiReturn_SUCCESS)
			{
				Filepath texturePath = std::format("{0}/{1}{2}", pathToParent, pathToTexture, path.C_Str());
				SceneData.TexturesToLoad.push_back({ materialIdx, { texturePath, ETextureType::MetallicRoughness } });
			}

			if (assimpMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &path) == aiReturn_SUCCESS)
			{
				Filepath texturePath = std::format("{0}/{1}{2}", pathToParent, pathToTexture, path.C_Str());
				SceneData.TexturesToLoad.push_back({ materialIdx, { texturePath, ETextureType::Emissive } });
			}

			assimpMaterial->Get(AI_MATKEY_METALLIC_FACTOR,		material.Metallic);
			assimpMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR,		material.Roughness);
			assimpMaterial->Get(AI_MATKEY_GLTF_ALPHACUTOFF,		material.AlphaCutoff);
			assimpMaterial->Get(AI_MATKEY_REFRACTI,				material.IndexOfRefraction);
			assimpMaterial->Get(AI_MATKEY_ANISOTROPY_FACTOR,	material.Anisotropy);
			assimpMaterial->Get(AI_MATKEY_GLOSSINESS_FACTOR,	material.Glossiness);
			assimpMaterial->Get(AI_MATKEY_REFLECTIVITY,			material.Reflectivity);

			aiColor4D baseColorFactor{};
			assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, baseColorFactor);
			material.BaseColorFactor = *(DirectX::XMFLOAT4*)(&baseColorFactor);

			aiColor4D emissiveColorFactor{};
			assimpMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColorFactor);
			material.EmissiveFactor = *(DirectX::XMFLOAT4*)(&emissiveColorFactor);

			aiString blend;
			assimpMaterial->Get(AI_MATKEY_GLTF_ALPHAMODE, blend);

			if (std::strcmp(blend.C_Str(), "OPAQUE") == 0)
			{
				material.AlphaMode = EAlphaMode::Opaque;
			}
			else if (std::strcmp(blend.C_Str(), "BLEND") == 0)
			{
				material.AlphaMode = EAlphaMode::Blend;
			}
			else if (std::strcmp(blend.C_Str(), "MASK") == 0)
			{
				material.AlphaMode = EAlphaMode::Mask;
			}

			SceneData.UniqueMaterials.push_back(material);
		}
	}

	bool IsTextureLoaded(assimp::FAssimpLoadingData& SceneData, Filepath Path)
	{
		return std::find(SceneData.LoadedPaths.begin(), SceneData.LoadedPaths.end(), Path.string()) != SceneData.LoadedPaths.end();
	}

	uint32 FindTextureWithPath(const std::vector<D3D12Texture*>& Textures, Filepath Path)
	{
		for (uint32 textureIdx = 0; textureIdx < Textures.size(); ++textureIdx)
		{
			if (Textures.at(textureIdx)->GetFilepath() == Path)
			{
				return textureIdx;
			}
		}

		return 0xFFFFFFFF;
	}

} // namespace Luden

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

			std::vector<StaticMesh> Meshes;

			//std::vector<aiMaterial*>	UniqueAssimpMaterials;
			std::vector<Material>		UniqueMaterials;
			std::vector<D3D12Texture*>	ModelTextures;

			// Temp
			std::vector<std::string> LoadedPaths;

			//std::map<> TextureToMaterial;

			// Currently not using any animation, thus nodes aren't needed at all.
			//std::vector<FNode*>	Nodes;
		};

	} // namespace assimp

	// Process Assimp node recursively.
	// Get information about it's meshes, matrix transformation and material.
	static void TraverseNode(FAssimpLoadingData& SceneData, aiNode* pNode);

	// Load Texture from given path, push it into loaded scene data and return it's SRV index.
		// It looks if texture has already been loaded, if so, returned index is from that texture.
	static uint32 FindTextureWithPath(const std::vector<D3D12Texture*>& Textures, Filepath Path);

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

		FAssimpLoadingData data{};
		data.Scene = scene;
		data.Path = Path;

		LoadMaterials(data);
		TraverseNode(data, scene->mRootNode);

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

	void AssetImporter::LoadMaterials(FAssimpLoadingData& SceneData)
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

				if (!IsTextureLoaded(SceneData, texturePath))
				{
					D3D12Texture* baseColorTexture = LoadTexture(texturePath);
					material.BaseColorIndex = baseColorTexture->ShaderResourceHandle.Index;
					baseColorTexture->SetFilepath(texturePath);
					SceneData.ModelTextures.push_back(std::move(baseColorTexture));

					SceneData.LoadedPaths.push_back(texturePath.string());
				}
				else
				{
					auto index = FindTextureWithPath(SceneData.ModelTextures, texturePath);
					material.BaseColorIndex = SceneData.ModelTextures.at(index)->ShaderResourceHandle.Index;
				}
			}

			if (assimpMaterial->GetTexture(aiTextureType_NORMALS, 0, &path) == aiReturn_SUCCESS)
			{
				Filepath texturePath = std::format("{0}/{1}{2}", pathToParent, pathToTexture, path.C_Str());

				if (!IsTextureLoaded(SceneData, texturePath))
				{
					D3D12Texture* normalTexture = LoadTexture(texturePath);
					material.NormalIndex = normalTexture->ShaderResourceHandle.Index;
					normalTexture->SetFilepath(texturePath);
					SceneData.ModelTextures.push_back(std::move(normalTexture));

					SceneData.LoadedPaths.push_back(texturePath.string());
				}
				else
				{
					auto index = FindTextureWithPath(SceneData.ModelTextures, texturePath);
					material.NormalIndex = SceneData.ModelTextures.at(index)->ShaderResourceHandle.Index;
				}
			}

			if (assimpMaterial->GetTexture(aiTextureType_METALNESS, 0, &path) == aiReturn_SUCCESS)
			{
				Filepath texturePath = std::format("{0}/{1}{2}", pathToParent, pathToTexture, path.C_Str());

				if (!IsTextureLoaded(SceneData, texturePath))
				{
					D3D12Texture* metallicTexture = LoadTexture(texturePath);
					material.MetallicRoughnessIndex = metallicTexture->ShaderResourceHandle.Index;
					metallicTexture->SetFilepath(texturePath);
					SceneData.ModelTextures.push_back(std::move(metallicTexture));

					SceneData.LoadedPaths.push_back(texturePath.string());
				}
				else
				{
					auto index = FindTextureWithPath(SceneData.ModelTextures, texturePath);
					material.MetallicRoughnessIndex = SceneData.ModelTextures.at(index)->ShaderResourceHandle.Index;
				}
			}

			if (assimpMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &path) == aiReturn_SUCCESS)
			{
				Filepath texturePath = std::format("{0}/{1}{2}", pathToParent, pathToTexture, path.C_Str());

				if (!IsTextureLoaded(SceneData, texturePath))
				{
					D3D12Texture* emissiveTexture = LoadTexture(texturePath);
					material.EmissiveIndex = emissiveTexture->ShaderResourceHandle.Index;
					emissiveTexture->SetFilepath(texturePath);
					SceneData.ModelTextures.push_back(std::move(emissiveTexture));

					SceneData.LoadedPaths.push_back(texturePath.string());
				}
				else
				{
					auto index = FindTextureWithPath(SceneData.ModelTextures, texturePath);
					material.EmissiveIndex = SceneData.ModelTextures.at(index)->ShaderResourceHandle.Index;
				}
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
			//SceneData.UniqueAssimpMaterials.push_back(assimpMaterial);
		}
	}

	//AssetImporter::
	void TraverseNode(FAssimpLoadingData& SceneData, aiNode* pNode)
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

	bool AssetImporter::IsTextureLoaded(FAssimpLoadingData& SceneData, Filepath Path)
	{
		for (usize idx = 0; idx < SceneData.LoadedPaths.size(); ++idx)
		{
			if (Path == SceneData.LoadedPaths.at(idx))
			{
				return true;
			}
		}

		return false;
	}

	uint32 FindTextureWithPath(const std::vector<D3D12Texture*>& Textures, Filepath Path)
	{

		for (uint32 textureIdx = 0; textureIdx < Textures.size(); ++textureIdx)
		{
			auto& texture = Textures.at(textureIdx);

			if (texture->GetFilepath() == Path)
			{
				return textureIdx;
			}
		}

		return 0xFFFFFFFF;
	}

	uint32 LoadTextureFromPath(FAssimpLoadingData& SceneData, Filepath Path)
	{
		// Look if texture has already been loaded.
		bool bIsLoaded = false;
		for (usize idx = 0; idx < SceneData.LoadedPaths.size(); ++idx)
		{
			if (Path == SceneData.LoadedPaths.at(idx))
			{
				bIsLoaded = true;
				break;
				//return true;
			}
		}

		if (bIsLoaded)
		{
			auto index = FindTextureWithPath(SceneData.ModelTextures, Path);
			return SceneData.ModelTextures.at(index)->ShaderResourceHandle.Index;
		}

		//D3D12Texture* emissiveTexture = LoadTexture(texturePath);
		//material.EmissiveIndex = emissiveTexture->ShaderResourceHandle.Index;
		//emissiveTexture->SetFilepath(texturePath);
		//SceneData.ModelTextures.push_back(std::move(emissiveTexture));
		//
		//SceneData.LoadedPaths.push_back(texturePath.string());


		return 0xFFFFFFFF;
	}

} // namespace Luden

#define _CRT_SECURE_NO_WARNINGS
#include "Graphics/Model.hpp"
#include "AssetImporter.hpp"
#include <Core/Logger.hpp>

#include <assimp/GltfMaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>

#include <meshoptimizer/meshoptimizer.h>

#include <DirectXTex.h>


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
			aiProcess_PreTransformVertices |
			//aiProcess_ImproveCacheLocality |
			//aiProcess_OptimizeMeshes |
			aiProcess_GenBoundingBoxes;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Path.string(), (uint32)loadFlags);

		if (!scene || !scene->mRootNode || !scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			LOG_WARNING("\n\tFailed to load model: {0}, reason: {1}", scene->GetShortFilename(path.c_str()), importer.GetErrorString());
			
			importer.FreeScene();

			return;
		}

		OutModel.Meshes.reserve(scene->mNumMeshes);

		LoadStaticMesh(scene, OutModel);
		LoadMaterials(scene, OutModel);


		importer.FreeScene();

	}	

	void AssetImporter::ImportStaticMesh(Filepath Path, Model& OutModel, bool bCGLTF)
	{
		const std::string& path = Path.string();
		if (!File::Exists(Path))
		{
			LOG_WARNING("{0} path is invalid.", path);

			return;
		}

		cgltf_options options{};
		cgltf_data* data = nullptr;
		cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
		if (result != cgltf_result_success)
		{
			LOG_ERROR("Failed to cgltf_parse_file!");
		
			cgltf_free(data);
		}

		result = cgltf_load_buffers(&options, data, path.c_str());
		if (result != cgltf_result_success)
		{
			LOG_ERROR("Failed to load glTF buffers!");
		}

		result = cgltf_validate(data);
		if (result != cgltf_result_success)
		{
			LOG_ERROR("Failed to validate glTF data!");
		}

		OutModel.Meshes.reserve(data->meshes_count);

		LoadStaticMesh(data, OutModel);

		cgltf_free(data);

	}

	void AssetImporter::LoadTextureDDS(Filepath Path)
	{	
		DirectX::ScratchImage scratchImage{};

		if (FAILED(DirectX::LoadFromDDSFile(Path.c_str(), DirectX::DDS_FLAGS_ALLOW_LARGE_FILES, nullptr, scratchImage)))
		{

		}

		DirectX::TexMetadata metadata = scratchImage.GetMetadata();
		
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

			meshData.Vertices.reserve(mesh->mNumVertices);

			for (uint32_t vertexId = 0; vertexId < mesh->mNumVertices; ++vertexId)
			{
				Vertex vertex{};

				if (mesh->HasPositions())
				{
					vertex.Position = *(DirectX::XMFLOAT3*)(&mesh->mVertices[vertexId]);
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

			BuildMesh(meshData);

			/*
			DirectX::ComputeMeshlets(
				meshData.Indices.data(), meshData.Indices.size() / 3,
				positions.data(), meshData.Vertices.size(),
				nullptr,
				meshData.Meshlets, meshData.MeshletVertices,
				meshData.MeshletTriangles,
				MeshletMaxVertices, MeshletMaxTriangles);

			auto uniqueVertexIndices = reinterpret_cast<const uint16_t*>(meshData.MeshletVertices.data());
			size_t vertIndices = meshData.MeshletVertices.size() / sizeof(uint16_t);

			meshData.MeshletCull.resize(meshData.Meshlets.size());
			if (FAILED(DirectX::ComputeCullData(
				positions.data(), positions.size(),
				meshData.Meshlets.data(), meshData.Meshlets.size(),
				uniqueVertexIndices, vertIndices,
				meshData.MeshletTriangles.data(), meshData.MeshletTriangles.size(),
				meshData.MeshletCull.data(), DirectX::MESHLET_FLAGS::MESHLET_DEFAULT)))
			{
				LOG_ERROR("DirectX::ComputeCullData");
			}
			
			*/

			meshData.NumVertices	= static_cast<uint32>(meshData.Vertices.size());
			meshData.NumIndices		= static_cast<uint32>(meshData.Indices.size());
			meshData.NumMeshlets	= static_cast<uint32>(meshData.Meshlets.size());

			meshData.MaterialIndex = mesh->mMaterialIndex;

			OutModel.Meshes.push_back(meshData);
		}
	}

	void AssetImporter::LoadMaterials(const aiScene* pScene, Model& OutModel)
	{
		for (auto& mesh : OutModel.Meshes)
		{
			aiMaterial* material = pScene->mMaterials[mesh.MaterialIndex];
			//material->

			Material newMaterial{};

			aiString path{};

			if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == aiReturn_SUCCESS || 
				material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
			{
				aiColor4D colorFactor{};
				aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &colorFactor);
				newMaterial.BaseColorFactor = *(DirectX::XMFLOAT4*)(&colorFactor);

			}

			if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == aiReturn_SUCCESS)
			{

			}

			if (material->GetTexture(aiTextureType_METALNESS, 0, &path) == aiReturn_SUCCESS)
			{

			}

			if (material->GetTexture(aiTextureType_EMISSIVE, 0, &path) == aiReturn_SUCCESS)
			{
				aiColor4D colorFactor{};
				aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &colorFactor);
				newMaterial.EmissiveFactor = *(DirectX::XMFLOAT4*)(&colorFactor);

			}

			if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &path) == aiReturn_SUCCESS)
			{

			}

			aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR,		&newMaterial.Metallic);
			aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR,	&newMaterial.Roughness);
			aiGetMaterialFloat(material, AI_MATKEY_GLTF_ALPHACUTOFF,	&newMaterial.AlphaCutoff);
			aiGetMaterialFloat(material, AI_MATKEY_SPECULAR_FACTOR,		&newMaterial.Specular);

			OutModel.Materials.emplace_back(newMaterial);

		}

	}

	void AssetImporter::BuildMesh(StaticMesh& Mesh)
	{
		/* =============================== Mesh optimizations ===============================*/

		std::vector<uint32> remap(Mesh.Indices.size());
		usize vertexCount = meshopt_generateVertexRemap(remap.data(), Mesh.Indices.data(), Mesh.Indices.size(), Mesh.Vertices.data(), Mesh.Vertices.size(), sizeof(Vertex));

		std::vector<uint32> remapIndices(Mesh.Indices.size());
		std::vector<Vertex> remapVertices(vertexCount);

		meshopt_remapIndexBuffer(remapIndices.data(), Mesh.Indices.data(), Mesh.Indices.size(), remap.data());
		meshopt_remapVertexBuffer(remapVertices.data(), Mesh.Vertices.data(), Mesh.Vertices.size(), sizeof(Vertex), remap.data());

		meshopt_optimizeVertexCache(remapIndices.data(), remapIndices.data(), remapIndices.size(), vertexCount);
		meshopt_optimizeOverdraw(remapIndices.data(), remapIndices.data(), remapIndices.size(), &(remapVertices[0].Position.x), vertexCount, sizeof(Vertex), 1.05f);
		meshopt_optimizeVertexFetch(remapVertices.data(), remapIndices.data(), remapIndices.size(), remapVertices.data(), remapVertices.size(), sizeof(Vertex));

		Mesh.Indices = remapIndices;
		Mesh.Vertices = remapVertices;

		/* =============================== Meshlet generatation ===============================*/

		size_t maxMeshlets = meshopt_buildMeshletsBound(Mesh.Indices.size(), MeshletMaxVertices, MeshletMaxTriangles);
		Mesh.Meshlets.resize(maxMeshlets);
		Mesh.MeshletVertices.resize(maxMeshlets * MeshletMaxVertices);
		Mesh.MeshletTriangles.resize(maxMeshlets * MeshletMaxTriangles * 3);

		const f32 coneWeight = 0.0f;

		size_t meshletCount = meshopt_buildMeshlets(
			Mesh.Meshlets.data(), Mesh.MeshletVertices.data(), Mesh.MeshletTriangles.data(),
			Mesh.Indices.data(), Mesh.Indices.size(),
			&Mesh.Vertices[0].Position.x, Mesh.Vertices.size(), sizeof(Vertex),
			MeshletMaxVertices, MeshletMaxTriangles,
			coneWeight);

		const meshopt_Meshlet& last = Mesh.Meshlets[meshletCount - 1];
		Mesh.MeshletVertices.resize(last.vertex_offset + (size_t)last.vertex_count);
		Mesh.MeshletTriangles.resize(last.triangle_offset + (size_t)((last.triangle_count * 3 + 3) & ~3));
		Mesh.Meshlets.resize(meshletCount);

		/* =============================== Meshlet cull data generation =============================== */

		Mesh.MeshletBounds.reserve(Mesh.Meshlets.size());

		for (uint32 m = 0; m < Mesh.Meshlets.size(); ++m)
		{
			meshopt_Meshlet& meshlet = Mesh.Meshlets.at(m);

			// Experimental
			meshopt_optimizeMeshlet(&Mesh.MeshletVertices[meshlet.vertex_offset], &Mesh.MeshletTriangles[meshlet.triangle_offset], meshlet.triangle_count, meshlet.vertex_count);

			// Generate bounds
			meshopt_Bounds bounds = meshopt_computeMeshletBounds(&Mesh.MeshletVertices[meshlet.vertex_offset], &Mesh.MeshletTriangles[meshlet.triangle_offset],
				meshlet.triangle_count, &Mesh.Vertices[0].Position.x, Mesh.Vertices.size(), sizeof(Vertex));

			FMeshletBounds output{
				.Center = *(DirectX::XMFLOAT3*)(&bounds.center),
				.Radius = bounds.radius,
				.ConeApex = *(DirectX::XMFLOAT3*)(&bounds.cone_apex),
				.ConeAxis = *(DirectX::XMFLOAT3*)(&bounds.cone_axis),
				.ConeCutoff = bounds.cone_cutoff
			};	

			Mesh.MeshletBounds.emplace_back(output);
		}


	}

	void AssetImporter::LoadStreamedStaticMesh(const aiScene* pScene, Model& OutModel, bool test)
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

			meshData.VertexStream.Positions.reserve(mesh->mNumVertices);
			meshData.VertexStream.TexCoords.reserve(mesh->mNumVertices);
			meshData.VertexStream.Normals.reserve(mesh->mNumVertices);
			meshData.VertexStream.Tangents.reserve(mesh->mNumVertices);

			for (uint32_t vertexId = 0; vertexId < mesh->mNumVertices; ++vertexId)
			{
				if (mesh->HasPositions())
				{
					meshData.VertexStream.Positions.emplace_back(*(DirectX::XMFLOAT3*)(&mesh->mVertices[vertexId]));
				}

				if (mesh->HasTextureCoords(0))
				{
					meshData.VertexStream.TexCoords.emplace_back(*(DirectX::XMFLOAT2*)(&mesh->mTextureCoords[0][vertexId]));
				}

				if (mesh->HasNormals())
				{
					meshData.VertexStream.Normals.emplace_back(*(DirectX::XMFLOAT3*)(&mesh->mNormals[vertexId]));
				}

				if (mesh->HasTangentsAndBitangents())
				{
					meshData.VertexStream.Tangents.emplace_back(*(DirectX::XMFLOAT4*)(&mesh->mTangents[vertexId]));
				}
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

			//DirectX::ComputeMeshlets(
			//	meshData.Indices.data(), meshData.Indices.size() / 3,
			//	meshData.VertexStream.Positions.data(), meshData.Vertices.size(),
			//	nullptr,
			//	meshData.Meshlets, meshData.MeshletVertices,
			//	meshData.MeshletTriangles,
			//	MeshletMaxVertices, MeshletMaxTriangles);


			meshData.NumVertices	= static_cast<uint32>(meshData.VertexStream.Positions.size());
			meshData.NumIndices		= static_cast<uint32>(meshData.Indices.size());
			meshData.NumMeshlets	= static_cast<uint32>(meshData.Meshlets.size());

			meshData.PositionOffset = 0;
			meshData.TexCoordOffset = meshData.PositionOffset	+ static_cast<uint32>(meshData.NumVertices * sizeof(DirectX::XMFLOAT3));
			meshData.NormalOffset	= meshData.TexCoordOffset	+ static_cast<uint32>(meshData.NumVertices * sizeof(DirectX::XMFLOAT2));
			meshData.TangentOffset	= meshData.NormalOffset		+ static_cast<uint32>(meshData.NumVertices * sizeof(DirectX::XMFLOAT3));

			OutModel.Meshes.push_back(meshData);
		}
	}

	void AssetImporter::LoadStaticMesh(const cgltf_data* pScene, Model& OutModel)
	{
		// Iterate over all meshes in 
		for (uint32 meshIdx = 0; meshIdx < pScene->meshes_count; ++meshIdx)
		{
			const cgltf_mesh& mesh = pScene->meshes[meshIdx];
			
			for (uint32 primitiveIdx = 0; primitiveIdx < mesh.primitives_count; ++primitiveIdx)
			{
				const cgltf_primitive& primitive = mesh.primitives[primitiveIdx];

				StaticMesh meshData{};

				std::vector<DirectX::XMFLOAT3> positions;
				std::vector<DirectX::XMFLOAT2> texCoords;
				std::vector<DirectX::XMFLOAT3> normals;
				std::vector<DirectX::XMFLOAT4> tangents;

				const cgltf_accessor* indexAccessor = primitive.indices;
				meshData.Indices.reserve(indexAccessor->count);

				for (uint64 idx = 0; idx < indexAccessor->count; idx += 3)
				{
					meshData.Indices.push_back((uint32)cgltf_accessor_read_index(indexAccessor, idx + 0));
					meshData.Indices.push_back((uint32)cgltf_accessor_read_index(indexAccessor, idx + 1));
					meshData.Indices.push_back((uint32)cgltf_accessor_read_index(indexAccessor, idx + 2));
				}

				for (uint32 attribIdx = 0; attribIdx < primitive.attributes_count; ++attribIdx)
				{
					const cgltf_attribute& attribute = primitive.attributes[attribIdx];
					
					for (uint32 vertexIdx = 0; vertexIdx < attribute.data->count; ++vertexIdx)
					{
						if (attribute.type == cgltf_attribute_type_position)
						{
							DirectX::XMFLOAT3 position{};
							cgltf_accessor_read_float(attribute.data, vertexIdx, (cgltf_float*)&position, 3);
							positions.emplace_back(position);
						}
						else if (attribute.type == cgltf_attribute_type_texcoord)
						{
							DirectX::XMFLOAT2 texCoord{};
							cgltf_accessor_read_float(attribute.data, vertexIdx, (cgltf_float*)&texCoord, 2);
							texCoords.emplace_back(texCoord);
						}
						else if (attribute.type == cgltf_attribute_type_normal)
						{
							DirectX::XMFLOAT3 normal{};
							cgltf_accessor_read_float(attribute.data, vertexIdx, (cgltf_float*)&normal, 3);
							normals.emplace_back(normal);
						}
						else if (attribute.type == cgltf_attribute_type_tangent)
						{
							DirectX::XMFLOAT4 tangent{};
							cgltf_accessor_read_float(attribute.data, vertexIdx, (cgltf_float*)&tangent, 4);
							tangents.emplace_back(tangent);
						}
					}
				}

				usize vertexCount = positions.size();

				if (texCoords.size() != vertexCount)	texCoords.resize(vertexCount);
				if (normals.size() != vertexCount)		normals.resize(vertexCount);
				if (tangents.size() != vertexCount)		tangents.resize(vertexCount);

				for (uint32 vertexIdx = 0; vertexIdx < positions.size(); ++vertexIdx)
				{
					Vertex vertex{};
					vertex.Position = positions.at(vertexIdx);
					vertex.TexCoord = texCoords.at(vertexIdx);
					vertex.Normal	= normals.at(vertexIdx);
					vertex.Tangent	= tangents.at(vertexIdx);

					meshData.Vertices.emplace_back(vertex);
				}

				BuildMesh(meshData);
				
				//DirectX::ComputeMeshlets(
				//	meshData.Indices.data(), meshData.Indices.size() / 3,
				//	positions.data(), meshData.Vertices.size(),
				//	nullptr,
				//	meshData.Meshlets, meshData.MeshletVertices,
				//	meshData.MeshletTriangles,
				//	MeshletMaxVertices, MeshletMaxTriangles);

				meshData.NumVertices = static_cast<uint32>(meshData.Vertices.size());
				meshData.NumIndices = static_cast<uint32>(meshData.Indices.size());
				meshData.NumMeshlets = static_cast<uint32>(meshData.Meshlets.size());

				OutModel.Meshes.emplace_back(meshData);

			}
		}
	}

	
} // namespace Luden

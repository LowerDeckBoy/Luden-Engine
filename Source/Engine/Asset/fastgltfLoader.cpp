#include "AssetImporter.hpp"
#include <Core/Logger.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/dxmath_element_traits.hpp>
#include <fastgltf/types.hpp>
#include <map>

namespace Luden::glTF
{
	enum class ETextureType
	{
		BaseColor,
		Normal,
		MetallicRoughness,
		Emissive
	};

	struct FfastgltfLoadingData
	{
		fastgltf::Asset* Scene;
		Filepath Path;

		std::vector<StaticMesh> Meshes;
		std::vector<Material>	Materials;
		//std::vector<Material>	Materials;

		std::vector<std::pair<usize, ETextureType>> UniqueTextures;

		std::map<std::string, std::pair<usize, ETextureType>> MaterialToTexture;

		// Texture to material id?
		// Since there are no duplicated texture files it should work.
		// std::pair<Texture index, Type of texture>, uint32 - material index.
		std::map<std::pair<usize, ETextureType>, uint32> TextureToMaterial;
	};

	static void TraverseHierarchy(FfastgltfLoadingData& SceneData);

	static void LoadMaterials(FfastgltfLoadingData& SceneData);

} // namespace Luden::glTF

namespace Luden
{
	bool AssetImporter::ImportFastglftModel(Filepath Path, Model& OutModel)
	{

		fastgltf::Options gltfOptions = fastgltf::Options::LoadExternalBuffers;// | fastgltf::Options::LoadExternalImages;

		fastgltf::Parser parser{};

		auto data = fastgltf::GltfDataBuffer::FromPath(Path);
		if (data.error() != fastgltf::Error::None)
		{
			DEBUGBREAK();
			return false;
		}

		auto scene = parser.loadGltf(data.get(), File::GetParentPath(Path), gltfOptions);
		if (scene.error() != fastgltf::Error::None)
		{
			DEBUGBREAK();
			return false;
		}

		glTF::FfastgltfLoadingData loadData{};
		loadData.Scene = std::move(&scene.get());

		// https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-2/chapter-6/vk_loader.cpp
		// https://github.com/stripe2933/vk-gltf-viewer/blob/master/interface/gltf/algorithm/traversal.cppm

		glTF::TraverseHierarchy(loadData);
		glTF::LoadMaterials(loadData);

		for (auto& mesh : loadData.Meshes)
		{
			BuildMesh(mesh);
		}

		const std::string pathToParent = std::filesystem::absolute(Path).parent_path().string();
		// Some of glTF models use place their textures inside *textures/* directory, but some just don't.
		const std::string pathToTexture = (std::filesystem::exists("textures/") ? "textures/" : "");

		for (auto& [texture, material] : loadData.TextureToMaterial)
		{
			auto& gltfTexture = loadData.Scene->textures.at(texture.first);
			auto& image = loadData.Scene->images.at(gltfTexture.imageIndex.value());

			auto& uri = std::get<fastgltf::sources::URI>(image.data);
			D3D12Texture* tex2D = LoadTexture(std::format("{}/{}{}", pathToParent, pathToTexture, uri.uri.c_str()));

			switch (texture.second)
			{
			case glTF::ETextureType::BaseColor:
				loadData.Materials.at(material).BaseColorIndex = tex2D->ShaderResourceHandle.Index;
				break;
			case glTF::ETextureType::Normal:
				loadData.Materials.at(material).NormalIndex = tex2D->ShaderResourceHandle.Index;
				break;
			case glTF::ETextureType::MetallicRoughness:
				loadData.Materials.at(material).MetallicRoughnessIndex = tex2D->ShaderResourceHandle.Index;
				break;
			case glTF::ETextureType::Emissive:
				loadData.Materials.at(material).EmissiveIndex = tex2D->ShaderResourceHandle.Index;
				break;
			}

			OutModel.Textures.push_back(tex2D);			
		}

		OutModel.SetFilepath(Path);

		OutModel.Meshes = std::move(loadData.Meshes);
		OutModel.Materials = std::move(loadData.Materials);

		return true;
	}

	void glTF::TraverseHierarchy(FfastgltfLoadingData& SceneData)
	{
		const auto asset = SceneData.Scene;

		for (usize sceneIdx = 0; sceneIdx < asset->scenes.size(); ++sceneIdx)
		{
			fastgltf::iterateSceneNodes(*asset, sceneIdx, fastgltf::math::fmat4x4(),
				[&](fastgltf::Node& node, fastgltf::math::fmat4x4 matrix) {
					
					if (node.meshIndex.has_value())
					{
						const auto gltfMesh = &SceneData.Scene->meshes.at(node.meshIndex.value());

						for (usize primIdx = 0; primIdx < gltfMesh->primitives.size(); ++primIdx)
						{
							const fastgltf::Primitive* primitive = &gltfMesh->primitives.at(primIdx);

							StaticMesh meshData{};
							meshData.Transform.WorldMatrix = *(DirectX::XMMATRIX*)&matrix;

							if (primitive->indicesAccessor.has_value())
							{
								
								fastgltf::Accessor& indexAccessor = asset->accessors.at(primitive->indicesAccessor.value());
								meshData.Indices.reserve(indexAccessor.count);
								
								fastgltf::iterateAccessor<uint32>(*asset, indexAccessor,
									[&](uint32 index) {
										meshData.Indices.push_back(index);
									});
							}

							std::vector<DirectX::XMFLOAT4> positions;
							std::vector<DirectX::XMFLOAT2> texCoords;
							std::vector<DirectX::XMFLOAT3> normals;
							// TODO:
							//std::vector<DirectX::XMFLOAT4> tangents;
							//std::vector<DirectX::XMFLOAT4> bitangents;

							auto* positionIt = primitive->findAttribute("POSITION");
							if (asset->accessors[positionIt->accessorIndex].bufferViewIndex.has_value())
							{
								auto& accessor = asset->accessors[positionIt->accessorIndex];
								positions.reserve(accessor.count);
								fastgltf::iterateAccessor<DirectX::XMFLOAT3>(*asset, accessor,
									[&](DirectX::XMFLOAT3 vertex)
									{
										positions.push_back(DirectX::XMFLOAT4(vertex.x, vertex.y, vertex.z, 1.0f));
									});
							}

							auto* texCoordIt = primitive->findAttribute("TEXCOORD_0");
							if (asset->accessors[texCoordIt->accessorIndex].bufferViewIndex.has_value())
							{
								auto& accessor = asset->accessors[texCoordIt->accessorIndex];
								texCoords.reserve(accessor.count);
								fastgltf::iterateAccessor<DirectX::XMFLOAT2>(*asset, accessor,
									[&](DirectX::XMFLOAT2 uv) 
									{
										texCoords.push_back(uv);
									});

							}

							auto* normalIt = primitive->findAttribute("NORMAL");
							if (asset->accessors[normalIt->accessorIndex].bufferViewIndex.has_value())
							{
								auto& accessor = asset->accessors[normalIt->accessorIndex];
								normals.reserve(accessor.count);
								fastgltf::iterateAccessor<DirectX::XMFLOAT3>(*asset, accessor,
									[&](DirectX::XMFLOAT3 normal) 
									{
										normals.push_back(normal);
									});
							}

							meshData.MaterialId = (uint32)primitive->materialIndex.value();

							DirectX::XMVector4TransformStream(
								positions.data(), sizeof(DirectX::XMFLOAT4),
								positions.data(), sizeof(DirectX::XMFLOAT4),
								positions.size(), meshData.Transform.WorldMatrix);
							
							DirectX::XMVector3TransformNormalStream(
								normals.data(), sizeof(DirectX::XMFLOAT3),
								normals.data(), sizeof(DirectX::XMFLOAT3),
								normals.size(), meshData.Transform.WorldMatrix);

							DirectX::XMStoreFloat3x4(&meshData.RaytracingInstanceDesc.Transform, meshData.Transform.WorldMatrix);
							meshData.RaytracingInstanceDesc.InstanceID = 0;
							meshData.RaytracingInstanceDesc.InstanceMask = 1;

							meshData.Vertices.reserve(positions.size());
							for (usize vertIdx = 0; vertIdx < positions.size(); ++vertIdx)
							{
								Vertex vertex{};
								vertex.Position = *(DirectX::XMFLOAT3*)(&positions.at(vertIdx));
								vertex.TexCoord = texCoords.at(vertIdx);
								vertex.Normal	= normals.at(vertIdx);

								meshData.Vertices.push_back(vertex);
							}

							meshData.MaterialId = (primitive->materialIndex.has_value() ? static_cast<uint32>(primitive->materialIndex.value()) : 0xFFFFFFFF);

							SceneData.Meshes.push_back(meshData);
						}
					}
				});
		}

	}

	void glTF::LoadMaterials(FfastgltfLoadingData& SceneData)
	{
		auto asset = SceneData.Scene;

		uint32 index = 0;
		for (auto& gltfMaterial : asset->materials)
		{
			Material material{};
			
			material.BaseColorFactor.x	= gltfMaterial.pbrData.baseColorFactor.x();
			material.BaseColorFactor.y	= gltfMaterial.pbrData.baseColorFactor.y();
			material.BaseColorFactor.z	= gltfMaterial.pbrData.baseColorFactor.z();
			material.BaseColorFactor.w	= gltfMaterial.pbrData.baseColorFactor.w();

			material.EmissiveFactor.x	= gltfMaterial.emissiveFactor.x();
			material.EmissiveFactor.y	= gltfMaterial.emissiveFactor.y();
			material.EmissiveFactor.z	= gltfMaterial.emissiveFactor.z();
			material.EmissiveFactor.w	= gltfMaterial.emissiveStrength;

			material.AlphaCutoff		= gltfMaterial.alphaCutoff;
			material.Metallic			= gltfMaterial.pbrData.metallicFactor;
			material.Roughness			= gltfMaterial.pbrData.roughnessFactor;
	
			material.IndexOfRefraction	= gltfMaterial.ior;
			material.AlphaMode			= (EAlphaMode)gltfMaterial.alphaMode;

			SceneData.Materials.push_back(material);

			if (gltfMaterial.pbrData.baseColorTexture.has_value())
			{

				SceneData.UniqueTextures.push_back({ gltfMaterial.pbrData.baseColorTexture->textureIndex, ETextureType::BaseColor });
				SceneData.TextureToMaterial[{gltfMaterial.pbrData.baseColorTexture->textureIndex, ETextureType::BaseColor}] = index;
			}

			if (gltfMaterial.pbrData.metallicRoughnessTexture.has_value())
			{
				SceneData.UniqueTextures.push_back({ gltfMaterial.pbrData.metallicRoughnessTexture->textureIndex, ETextureType::MetallicRoughness });
				SceneData.TextureToMaterial[{ gltfMaterial.pbrData.metallicRoughnessTexture->textureIndex, ETextureType::MetallicRoughness }] = index;
			}

			if (gltfMaterial.normalTexture.has_value())
			{
				SceneData.UniqueTextures.push_back({ gltfMaterial.normalTexture->textureIndex, ETextureType::Normal });
				SceneData.TextureToMaterial[{ gltfMaterial.normalTexture->textureIndex, ETextureType::Normal }] = index;
			}

			if (gltfMaterial.emissiveTexture.has_value())
			{
				SceneData.UniqueTextures.push_back({ gltfMaterial.emissiveTexture->textureIndex, ETextureType::Emissive });
				SceneData.TextureToMaterial[{ gltfMaterial.emissiveTexture->textureIndex, ETextureType::Emissive }] = index;
			}

			++index;
		}

	}

} // namespace Luden

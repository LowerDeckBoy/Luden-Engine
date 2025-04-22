#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/dxmath_element_traits.hpp>

#include "AssetImporter.hpp"
#include <Core/Logger.hpp>


namespace Luden
{
	static void TraverseHierarchy(FfastgltfLoadingData& SceneData);

	bool AssetImporter::ImportFastglftModel(Filepath Path, Model& OutModel)
	{

		 fastgltf::Options gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::LoadExternalBuffers;// | fastgltf::Options::DecomposeNodeMatrices;

		fastgltf::Parser parser{};
		fastgltf::Asset gltf;

		auto data = fastgltf::GltfDataBuffer::FromPath(Path);
		if (data.error() != fastgltf::Error::None)
		{
			DEBUGBREAK();
		}

		auto scene = parser.loadGltf(data.get(), File::GetParentPath(Path), gltfOptions);
		if (scene.error() != fastgltf::Error::None)
		{
			DEBUGBREAK();
		}

		FfastgltfLoadingData loadData{};
		loadData.Scene = std::move(&scene.get());

		// https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-2/chapter-6/vk_loader.cpp
		// https://github.com/stripe2933/vk-gltf-viewer/blob/master/interface/gltf/algorithm/traversal.cppm

		TraverseHierarchy(loadData);

		for (auto& mesh : loadData.Meshes)
		{
			BuildMesh(mesh);
		}

		OutModel.SetFilepath(Path);

		OutModel.Meshes = std::move(loadData.Meshes);

		return true;
	}

	void TraverseHierarchy(FfastgltfLoadingData& SceneData)
	{
		const auto asset = SceneData.Scene;

		for (usize sceneIdx = 0; sceneIdx < asset->scenes.size(); ++sceneIdx)
		{
			auto& scene = asset->scenes.at(sceneIdx);
			
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
							//std::vector<DirectX::XMFLOAT4> tangents;
							//std::vector<DirectX::XMFLOAT4> bitangents;

							auto* positionIt = primitive->findAttribute("POSITION");
							if (asset->accessors[positionIt->accessorIndex].bufferViewIndex.has_value())
							{
								//positions.reserve(positionAccessor.count);
								fastgltf::iterateAccessor<DirectX::XMFLOAT3>(*asset, asset->accessors[positionIt->accessorIndex],
									[&](DirectX::XMFLOAT3 vertex) {

										positions.push_back(DirectX::XMFLOAT4(vertex.z, vertex.y, vertex.x, 1.0f));
										//positions.push_back(*(DirectX::XMFLOAT4*)&vertex);
									});
							}

							auto* texCoordIt = primitive->findAttribute("TEXCOORD_0");
							auto& texCoordAccessor = asset->accessors[texCoordIt->accessorIndex];
							if (texCoordAccessor.bufferViewIndex.has_value())
							{
								texCoords.reserve(texCoordAccessor.count);
								fastgltf::iterateAccessor<DirectX::XMFLOAT2>(*asset, texCoordAccessor,
									[&](DirectX::XMFLOAT2 uv) {
										texCoords.push_back(uv);
									});

							}

							auto* normalIt = primitive->findAttribute("NORMAL");
							auto& normalAccessor = asset->accessors[normalIt->accessorIndex];
							if (normalAccessor.bufferViewIndex.has_value())
							{
								normals.reserve(normalAccessor.count);
								fastgltf::iterateAccessor<DirectX::XMFLOAT3>(*asset, normalAccessor,
									[&](DirectX::XMFLOAT3 normal) {
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

							SceneData.Meshes.push_back(meshData);
						}
					}
				});
		}

	}

} // namespace Luden

#include "AssetImporter.hpp"
#include "D3D12/D3D12Memory.hpp"
#include "D3D12/D3D12UploadContext.hpp"
#include "D3D12/D3D12Utility.hpp"
#include <DirectXTex.h>
#include <meshoptimizer/meshoptimizer.h>


namespace Luden
{
	bool AssetImporter::ImportStaticMesh(Filepath Path, Model& OutModel)
	{
		if (!File::Exists(Path.string()))
		{
			LOG_WARNING("{0} path is invalid.", Path.string());

			return false;
		}
		
		if (File::GetExtension(Path) == ".gltf")
		{
			return ImportFastglftModel(Path, OutModel);
		}
		else
		{
			return ImportAssimpModel(Path, OutModel);
		}

		return false;
	}

	D3D12Texture* AssetImporter::LoadTexture(Filepath Path)
	{
		D3D12Texture* texture = new D3D12Texture();

		if (File::GetExtension(Path) == ".dds")
		{
			LoadTextureDDS(Path, texture);
		}
		else
		{
			LoadTexture2D(Path, texture);
		}

		return texture;
	}

	void AssetImporter::LoadTexture2D(Filepath Path, D3D12Texture* pTexture)
	{
		DirectX::ScratchImage scratchImage{};
		DirectX::TexMetadata metadata{};

		HRESULT result = DirectX::LoadFromWICFile(Path.wstring().c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage);
		
		if (FAILED(result))
		{
			char hResultError[512]{};
			::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, result, 0, hResultError, (sizeof(hResultError) / sizeof(char)), nullptr);
			LOG_WARNING("Failed to load texture: {}\n{}", Path.string(), hResultError);
			__debugbreak();
			return;
		}
		
		TextureDesc desc{};
		desc.Data			= (void*)&scratchImage.GetImages()->pixels[0];
		desc.Width			= static_cast<uint32>(metadata.width);
		desc.Height			= static_cast<uint32>(metadata.height);
		desc.DepthOrArray	= static_cast<uint16>(metadata.depth);
		desc.NumMips		= std::min((uint16)5, static_cast<uint16>(metadata.mipLevels));
		//desc.NumMips		= 1;
		desc.Format			= metadata.format;

		pTexture->Create(Device, desc);
		
		pTexture->Subresource.pData			= pTexture->GetTextureDesc().Data;
		pTexture->Subresource.RowPitch		= scratchImage.GetImages()->rowPitch;
		pTexture->Subresource.SlicePitch	= scratchImage.GetImages()->slicePitch;
		
		const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(pTexture->Subresource.SlicePitch);
		
		auto heapProperties = D3D::HeapPropertiesUpload();
		
		D3D12Resource* uploadResource = new D3D12Resource();
		
		VERIFY_D3D12_RESULT(Device->LogicalDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadResource->GetHandle())));
		
		uploadResource->SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
		uploadResource->SetDebugName("D3D12 Upload Texture Resource");
		
		D3D12UploadContext::UploadTexture(pTexture, uploadResource);

	}

	void AssetImporter::LoadTextureDDS(Filepath Path, D3D12Texture* pTexture)
	{	
		DirectX::ScratchImage scratchImage{};
		DirectX::TexMetadata metadata{};
		HRESULT result = DirectX::LoadFromDDSFile(Path.c_str(), DirectX::DDS_FLAGS_FORCE_RGB, &metadata, scratchImage);
		if (FAILED(result))
		{
			char hResultError[512]{};
			::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, result, 0, hResultError, (sizeof(hResultError) / sizeof(char)), nullptr);
			LOG_WARNING("Failed to load texture: {}", hResultError);
		}

		TextureDesc desc{};
		desc.Data			= (void*)scratchImage.GetImages()->pixels;
		desc.Width			= static_cast<uint32>(metadata.width);
		desc.Height			= static_cast<uint32>(metadata.height);
		desc.DepthOrArray	= static_cast<uint16>(metadata.depth);
		desc.NumMips		= static_cast<uint16>(metadata.mipLevels);
		desc.Format			= metadata.format;
		//desc.NumMips = 1;

		//DirectX::Decompress()

		pTexture->Create(Device, desc);

		pTexture->Subresource.pData = scratchImage.GetImages()->pixels;
		pTexture->Subresource.RowPitch = scratchImage.GetImages()->rowPitch;
		pTexture->Subresource.SlicePitch = scratchImage.GetImages()->slicePitch;

	}

	// Pack 3 uint8 triangle indices into single uint32 element.
	// Triangles are later unpacked in shader.
	// Note:
	// Currently not used here.
	static std::vector<uint32> RepackMeshletTriangles(std::vector<meshopt_Meshlet>& Meshlets, std::vector<uint8>& Triangles)
	{
		std::vector<uint32> meshletTrianglesRepacked;
		for (auto& meshlet : Meshlets)
		{
			uint32 triangleOffset = static_cast<uint32>(meshletTrianglesRepacked.size());

			for (uint32_t i = 0; i < meshlet.triangle_count; ++i)
			{
				uint32 i0 = 3 * i + 0 + meshlet.triangle_offset;
				uint32 i1 = 3 * i + 1 + meshlet.triangle_offset;
				uint32 i2 = 3 * i + 2 + meshlet.triangle_offset;

				uint8  idx0 = Triangles[i0];
				uint8  idx1 = Triangles[i1];
				uint8  idx2 = Triangles[i2];
				uint32 packed = 
					((static_cast<uint32>(idx0) & 0xFF)		)  |
					((static_cast<uint32>(idx1) & 0xFF) << 8)  |
					((static_cast<uint32>(idx2) & 0xFF) << 16);
				meshletTrianglesRepacked.push_back(packed);
			}

			meshlet.triangle_offset = triangleOffset;
		}

		return meshletTrianglesRepacked;
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
			Mesh.Meshlets.data(),
			Mesh.MeshletVertices.data(),
			Mesh.MeshletTriangles.data(),
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

		//Mesh.MeshletTriangles = RepackMeshletTriangles()

		Mesh.NumVertices = static_cast<uint32>(Mesh.Vertices.size());
		Mesh.NumIndices = static_cast<uint32>(Mesh.Indices.size());
		Mesh.NumMeshlets = static_cast<uint32>(Mesh.Meshlets.size());
		Mesh.NumMeshletVertices = static_cast<uint32>(Mesh.MeshletVertices.size());
		Mesh.NumMeshletTriangles = static_cast<uint32>(Mesh.MeshletTriangles.size());
	}

	/*
	void AssetImporter::TraverseNode(const cgltf_data* pScene, const cgltf_node* pNode, Model& OutModel, FNode* pParentNode, DirectX::XMMATRIX ParentMatrix)
	{
		FNode* newNode = new FNode();
		newNode->Parent = pParentNode;
		if (pNode->name)
		{
			newNode->Name = pNode->name;
		}

		if (pNode->has_matrix)
		{

			cgltf_node_transform_world(pNode, &*(float*)&newNode->LocalTransform.WorldMatrix);
			DirectX::XMMatrixTranspose(newNode->LocalTransform.WorldMatrix);
			//newNode->LocalTransform.WorldMatrix = *(DirectX::XMMATRIX*)(localm);
			//newNode->LocalTransform.WorldMatrix = *(DirectX::XMMATRIX*)(pNode->matrix);
			newNode->LocalTransform.Decompose(newNode->LocalTransform.WorldMatrix);
			newNode->LocalTransform.Rotation.x -= 90.0f;
		}
		else
		{


		}

		DirectX::XMFLOAT3 translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		if (pNode->has_scale)
		{
			scale = *(DirectX::XMFLOAT3*)(&pNode->scale);
			newNode->LocalTransform.Scale = scale;
		}

		if (pNode->has_rotation)
		{
			rotation = *(DirectX::XMFLOAT4*)(&pNode->rotation);
			//rotation.x -= 90.0f;
			newNode->LocalTransform.Rotation = rotation;
		}

		if (pNode->has_translation)
		{
			translation = *(DirectX::XMFLOAT3*)(&pNode->translation);
			newNode->LocalTransform.Translation = translation;
		}

		if (pNode->children_count > 0)
		{
			for (uint32 childIdx = 0; childIdx < pNode->children_count; ++childIdx)
			{
				cgltf_node* child = pNode->children[childIdx];
				TraverseNode(pScene, child, OutModel, newNode, newNode->Transform.WorldMatrix);
			}
		}

		if (pNode->mesh)
		{
			for (uint32 primitiveIdx = 0; primitiveIdx < pNode->mesh->primitives_count; ++primitiveIdx)
			{
				const cgltf_primitive& primitive = pNode->mesh->primitives[primitiveIdx];

				StaticMesh meshData{};

				meshData.Transform.WorldMatrix = newNode->Transform.WorldMatrix;
				//meshData.Transform.WorldMatrix = newNode->LocalTransform * OutModel.GetComponent<ecs::TransformComponent>().WorldMatrix;
				//meshData.Transform.WorldMatrix = OutModel.GetComponent<ecs::TransformComponent>().WorldMatrix * newNode->LocalTransform;
				//meshData.Transform.WorldMatrix = newNode->Transform;
				//meshData.Transform.WorldMatrix = newNode->Transform;

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
							//positions.emplace_back(position);
							positions.emplace_back(DirectX::XMFLOAT3{ position.x, position.y, -position.z });
						}
						else if (attribute.type == cgltf_attribute_type_texcoord)
						{
							DirectX::XMFLOAT2 texCoord{};
							cgltf_accessor_read_float(attribute.data, vertexIdx, (cgltf_float*)&texCoord, 2);
							texCoords.emplace_back(DirectX::XMFLOAT2{texCoord.x, -texCoord.y});
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
					vertex.Normal = normals.at(vertexIdx);
					vertex.Tangent = tangents.at(vertexIdx);

					meshData.Vertices.emplace_back(vertex);
				}

				BuildMesh(meshData);

				meshData.NumVertices = static_cast<uint32>(meshData.Vertices.size());
				meshData.NumIndices  = static_cast<uint32>(meshData.Indices.size());
				meshData.NumMeshlets = static_cast<uint32>(meshData.Meshlets.size());
			#if not BUILD_NODES
				OutModel.Meshes.emplace_back(meshData);
			#else
				newNode->Meshes.push_back(new StaticMesh(meshData));
			#endif
			}
		}

		if (pParentNode)
		{
			pParentNode->Children.push_back(newNode);
		}
		else
		{
			OutModel.Nodes.push_back(newNode);
		}
	}
	*/

} // namespace Luden

#pragma once

#include "Graphics/Model.hpp"
#include <Core/File.hpp>
#include <Core/Logger.hpp>
#include <Core/Types.hpp>

namespace Luden
{
	class D3D12Device;
	class D3D12Texture;
	class Model;

	struct StaticMesh;
	struct Material;
	struct TextureDesc;

	enum class ETextureType
	{
		BaseColor,
		Normal,
		MetallicRoughness,
		Emissive
	};
	
	class AssetImporter
	{
	public:
		AssetImporter() = default;
		~AssetImporter() = default;
		
		bool ImportStaticMesh(Filepath Path, Model& OutModel);

		D3D12Texture* LoadTexture(Filepath Path);

		void LoadTexture2D(Filepath Path, D3D12Texture* pTexture);
		void LoadTextureDDS(Filepath Path, D3D12Texture* pTexture);

		D3D12Device* Device;

	private:
		// Use meshoptimizer to optimize mesh data and generate meshlets.
		void BuildMesh(StaticMesh& Mesh);

		//void GenerateTangents();

		// For non glTF models.
		bool ImportAssimpModel(Filepath Path, Model& OutModel);

		// for glTF 2.0 models only.
		bool ImportFastglftModel(Filepath Path, Model& OutModel);

		// Check if texture with given path has already been loaded.
		// If so, find that texture instead.
		bool IsTexturePathLoaded(const std::vector<std::string>& TexturePaths, Filepath Path);

		// Looks if texture has already been loaded, if so, returned index is from that texture.
		uint32 FindTextureWithPath(const std::vector<D3D12Texture*>& Textures, Filepath Path);

	};
} // namespace Luden

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

		bool ImportAssimpModel(Filepath Path, Model& OutModel);

		bool ImportFastglftModel(Filepath Path, Model& OutModel);

	};
} // namespace Luden

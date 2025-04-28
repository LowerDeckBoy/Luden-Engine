#pragma once

#include <Core/String.hpp>
#include <Core/File.hpp>
#include "SceneCamera.hpp"
#include "Graphics/Model.hpp"
#include "ECS/World.hpp"


namespace Luden
{
	class AssetImporter;
	class Renderer;

	// TODO:
	// AssetImporter should be owned here.
	// Pointer to RHI should too?
	// Materials, Textures and Buffers should be stored inside a Scene?
	// ^ that way it should be easier to create resource buffers and manage scene cleaning.
	class Scene
	{
	public:
		Scene();
		Scene(AssetImporter* pAssetImporter);
		Scene(AssetImporter* pAssetImporter, D3D12Device* pDevice);
		~Scene();

		std::string Name;

		// Initialize Scene and create resources for scene content.
		void Build(D3D12Device* pDevice);

		// Release all the Scene resources.
		// Clear World registry.
		void Release();

		void CreateEntity(Entity& Target);

		bool AddModel(Filepath Path);

		World*			GetWorld()		{ return m_World;			}
		entt::registry* GetRegistry()	{ return m_World->Registry; }

		bool IsEmpty() const;
		

		// TODOs:
		//void AddPointLight();
		//void AddDirectionalLight();
		//void AddSpotLight();

		// Test
		// Scene resource.
		// They gonna be used to create buffers for GPU driven rendering.
		std::vector<std::unique_ptr<Model>> Models;
		std::vector<Material> Materials;
		std::vector<StaticMesh> Meshes;
		std::vector<D3D12Texture*> Textures;

		// Log scene stats, like amount of models, meshes, lights this scene has.
		//void LogDebugInfo();

	private:
		Filepath m_Filepath;

		World* m_World = nullptr;
		AssetImporter* m_AssetImporter = nullptr;

	};
} // namespace Luden

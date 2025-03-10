#pragma once

#include <Core/String.hpp>
#include <Core/File.hpp>
#include "SceneCamera.hpp"
#include "Graphics/Model.hpp"
#include "ECS/World.hpp"
#include "SceneCamera.hpp"

namespace Luden
{
	constexpr std::string_view SceneRootFolder	= "Scenes/";

	// Note:
	// Currently using hard-coded path to avoid copying Assets into project OutDir.
	constexpr std::string_view ModelsRootFolder = "../../Assets/Models/";

	class Scene
	{
	public:
		Scene();
		~Scene();

		//void Load();
		//void Save();

		// Draw all models in this scene.
		void Draw();

		std::string Name;

		std::vector<Model> Models;

		void CreateEntity(Entity& Target);

		World* GetWorld()
		{
			return m_World;
		}

		entt::registry* GetRegistry()
		{
			return m_World->Registry;
		}

	private:
		Filepath m_Filepath;

		World* m_World = nullptr;

		std::vector<StaticMesh> m_StaticMeshes;
		std::vector<Material> m_Materials;

	};
} // namespace Luden

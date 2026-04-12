#pragma once

namespace Luden
{
	class Scene;
	class Entity;
}

namespace Luden::Panel
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		~SceneHierarchyPanel() = default;

		void SetActiveScene(Scene* pScene, Renderer* pRenderer);

		void DrawPanel();

		Entity& GetSelectedEntity() 
		{ 
			return m_SelectedEntity;
		}
		
		inline void ResetSelection() 
		{ 
			m_SelectedEntity = {}; 
		}

		inline static uint64 DisplayImageAddress = 0;

	private:
		Scene* m_ActiveScene = nullptr;
		Entity m_SelectedEntity{};
		Renderer* m_Renderer = nullptr;

	};
} // namespace Luden::Panel
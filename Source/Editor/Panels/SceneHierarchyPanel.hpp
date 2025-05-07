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
		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void SetActiveScene(Scene* pScene);

		void DrawPanel();

		Entity& GetSelectedEntity() { return m_SelectedEntity; }
		
		void ResetSelection() { m_SelectedEntity = {}; }

	private:
		Scene* m_ActiveScene = nullptr;
		Entity m_SelectedEntity{};


	};
} // namespace Luden::Panel

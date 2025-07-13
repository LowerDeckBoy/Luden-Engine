#pragma once

namespace Luden
{
	class Entity;
	class Scene;
} // namespace Luden

namespace Luden::Panel
{
	// 
	class PropertyPanel
	{
	public:
		PropertyPanel();
		~PropertyPanel();

		void DrawEntity(Entity& Target);

		template<typename T, typename TFunc>
		void DrawComponent(Entity& Target, TFunc UI)
		{
			if (!Target.HasComponent<T>())
			{
				return;
			}

			auto& component = Target.GetComponent<T>();

			UI(component);
		}

	private:
		Scene* m_ActiveScene = nullptr;

	};
} // namespace Luden::Panel

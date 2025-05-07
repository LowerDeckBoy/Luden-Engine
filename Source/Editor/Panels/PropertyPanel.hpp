#pragma once

namespace Luden
{
	class Entity;
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

	};
} // namespace Luden::Panel

#include "../Components/Components.hpp"
#include <Engine/ECS/Entity.hpp>
#include "PropertyPanel.hpp"


namespace Luden::Panel
{
	PropertyPanel::PropertyPanel()
	{

	}

	PropertyPanel::~PropertyPanel()
	{

	}

	void PropertyPanel::DrawEntity(Entity& Target)
	{
		// Drawable entites should always have Name and Transform components.
		DrawComponent<ecs::NameComponent>(Target, [&](auto& component) { gui::DrawNameComponent(component); });
		DrawComponent<ecs::TransformComponent>(Target, [&](auto& component) { gui::DrawTransformComponent(component); });
		
		// Recursively draw

	}
} // namespace Luden::Panel

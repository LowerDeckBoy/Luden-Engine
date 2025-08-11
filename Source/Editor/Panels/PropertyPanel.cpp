#include "../Components/Components.hpp"
#include <Engine/ECS/Entity.hpp>
#include <Engine/Graphics/Model.hpp>
#include "PropertyPanel.hpp"
#include <ImGui/imgui.h>
#include <Engine/ECS/Components/LightComponent.hpp>

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
		// Drawable entites should always have Name component.
		DrawComponent<ecs::NameComponent>(Target, [&](auto& component) { 
				gui::DrawNameComponent(component); 
			});
		
		if (Target.HasComponent<ecs::TransformComponent>())
		{
			DrawComponent<ecs::TransformComponent>(Target, [&](auto& component) { 
				gui::DrawTransformComponent(component); 
			});
		}

		if (Target.HasComponent<ecs::PointLightComponent>())
		{
			DrawComponent<ecs::PointLightComponent>(Target, [&](auto& component) { 
				gui::DrawPointLightComponent(component); 
				});
		}

		if (Target.HasComponent<ecs::DirectionalLightComponent>())
		{
			DrawComponent<ecs::DirectionalLightComponent>(Target, [&](auto& component) {
				gui::DrawDirectionalLightComponent(component);
				});
		}

		if (Target.HasComponent<ecs::SpotLightComponent>())
		{
			DrawComponent<ecs::SpotLightComponent>(Target, [&](auto& component) {
				gui::DrawSpotLightComponent(component);
				});
		}

	}
} // namespace Luden::Panel

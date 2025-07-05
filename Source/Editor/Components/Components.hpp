#pragma once

#include <Core/String.hpp>
#include <DirectXMath.h>

#include <Engine/ECS/Components/NameComponent.hpp>
#include <Engine/ECS/Components/TransformComponent.hpp>
#include <Engine/ECS/Components/LightComponent.hpp>

namespace Luden
{
	class Entity;
} // namespace Luden

namespace Luden::gui
{
	namespace Math
	{
		extern bool DrawFloat3(std::string_view Label, DirectX::XMFLOAT3& Float3);

		extern void DrawFloat4(std::string_view Label, DirectX::XMFLOAT4& Float4);

		extern void EditColor3(std::string_view Label, DirectX::XMFLOAT3& Float3);

	} // namespace Math
		
	extern void DrawTransformComponent(ecs::TransformComponent& Component);

	extern void DrawNameComponent(ecs::NameComponent& Component);

	extern void DrawPointLightComponent(ecs::PointLightComponent& Component);

	extern void AddOrRemoveComponent(Entity& Target);

	extern void DrawEntityHierarchy(Entity& Target);

} // namespace Luden::gui

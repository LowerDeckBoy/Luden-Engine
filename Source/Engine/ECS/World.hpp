#pragma once

#include <EnTT/entt.hpp>
//#include "Entity.hpp"

namespace Luden
{
	using EntityHandle = entt::entity;
	using NullHandle = entt::null_t();

	class Entity;

	class World
	{
		friend class Entity;
	public:
		World();
		~World();

		// Create empty Entity.
		Entity CreateEntity();
		// Create Entity with NameComponent.
		Entity CreateEntity(std::string_view Name);

		// Create empty Entity.
		void CreateEntity(Entity* pEntity);
		// Create Entity with NameComponent.
		void CreateEntity(Entity* pEntity, std::string_view Name);

		// Clear entire Registry.
		void Clear();

		bool EntityExists(Entity& Target) const;

		void Remove(Entity& Target);

		entt::registry* Registry = nullptr;

	};
} // namespace Luden

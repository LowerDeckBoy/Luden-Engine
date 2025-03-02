#pragma once

#include <EnTT/entt.hpp>

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

		Entity CreateEntity();

		void CreateEntity(Entity* pEntity);

		entt::registry* Registry = nullptr;

	private:

	};
} // namespace Luden

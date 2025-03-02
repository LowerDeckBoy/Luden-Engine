#include "Entity.hpp"
#include "World.hpp"

namespace Luden
{
	World::World()
	{
		Registry = new entt::registry();
	}

	World::~World()
	{
		if (Registry)
		{
			Registry->clear();

			delete Registry;
			Registry = nullptr;
		}
	}

	Entity World::CreateEntity()
	{
		Entity entity{};
		entity.m_Handle = Registry->create();
		
		return entity;
	}

	void World::CreateEntity(Entity* pEntity)
	{
		//if (pEntity->IsValid())
		//{
		//	return;
		//}

		pEntity->m_Handle = Registry->create();
		pEntity->m_ParentWorld = this;
	}

} // namespace Luden

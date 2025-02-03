#include "Entity.hpp"
#include "World.hpp"

namespace Luden
{
	World::World()
	{
		m_Registry = new entt::registry();
	}

	World::~World()
	{
		if (m_Registry)
		{
			m_Registry->clear();

			delete m_Registry;

			m_Registry = nullptr;
		}
	}

	Entity World::CreateEntity()
	{
		Entity entity{};
		entity.m_Handle = m_Registry->create();
		
		return entity;
	}

	void World::CreateEntity(Entity* pEntity)
	{
		if (pEntity->IsValid())
		{
			return;
		}

		pEntity->m_Handle = m_Registry->create();
	}

} // namespace Luden

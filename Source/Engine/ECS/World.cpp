#include "Components/NameComponent.hpp"
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
			Clear();

			delete Registry;
			Registry = nullptr;
		}
	}

	Entity World::CreateEntity()
	{
		Entity entity(this, Registry->create());
		
		return entity;
	}

	Entity World::CreateEntity(std::string_view Name)
	{
		Entity entity(this, Registry->create());
		entity.AddComponent<ecs::NameComponent>(Name);

		return Entity();
	}

	void World::CreateEntity(Entity* pEntity)
	{
		pEntity->m_ParentWorld = this;
		pEntity->SetHandle(Registry->create());
	}

	void World::CreateEntity(Entity* pEntity, std::string_view Name)
	{
		pEntity->m_ParentWorld = this;
		pEntity->SetHandle(Registry->create());
		pEntity->AddComponent<ecs::NameComponent>(Name);
	}

	void World::Clear()
	{
		Registry->clear();
	}

	bool World::EntityExists(Entity& Target) const
	{
		return Registry->valid(Target.GetHandle());
	}

	void World::Remove(Entity& Target)
	{
		if (EntityExists(Target))
		{
			Registry->destroy(Target.GetHandle());
		}
	}

} // namespace Luden

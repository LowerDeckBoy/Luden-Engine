#include "Entity.hpp"

namespace Luden
{
	Entity::Entity(World* pWorld) 
		: m_ParentWorld(pWorld), m_Handle(entt::null)
	{
	}

	Entity::Entity(World* pWorld, entt::entity Handle)
		: m_ParentWorld(pWorld), m_Handle(Handle)
	{
	}

	Entity::Entity(const Entity& Other)
		:m_ParentWorld(Other.m_ParentWorld), m_Handle(Other.m_Handle)
	{
	}

	bool Entity::IsValid()
	{
		return m_ParentWorld->Registry->valid(m_Handle);
	}
	
	bool Entity::IsAlive()
	{
		return m_ParentWorld != nullptr;
	}
} // namespace Luden

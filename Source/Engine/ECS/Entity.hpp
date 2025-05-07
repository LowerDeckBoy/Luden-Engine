#pragma once

//#include <EnTT/entt.hpp>
#include "World.hpp"

namespace Luden
{
	class World;

	class Entity
	{
		friend class World;
	public:
		Entity() = default;
		Entity(World* pWorld);
		Entity(World* pWorld, entt::entity Handle);
		Entity(const Entity& Other);
		
		bool IsValid();
		bool IsAlive();

		entt::entity&	GetHandle()		 { return m_Handle; }
		World*			GetParentWorld() { return m_ParentWorld; }

		void SetHandle(entt::entity Handle) { m_Handle = Handle; }

		template<typename T>
		bool HasComponent()
		{
			return m_ParentWorld->Registry->any_of<T>(m_Handle);
		}
		
		template<typename T, typename... TArgs>
		void AddComponent(TArgs&&... Args)
		{
			m_ParentWorld->Registry->emplace<T>(m_Handle, std::forward<TArgs>(Args)...);
		}
		
		template<typename T>
		T& GetComponent()
		{
			return m_ParentWorld->Registry->get<T>(m_Handle);
		}
		
		template<typename T>
		void RemoveComponent()
		{
			if (HasComponent<T>())
			{
				m_ParentWorld->Registry->remove<T>(m_Handle);
			}
		}

		//void Destroy();

		bool operator==(const Entity& Other) const { return (m_Handle == Other.m_Handle); }

		bool HasParent() const { return Parent != nullptr; }

		Entity* Parent = nullptr;
		std::vector<Entity*> Children;

	protected:
		entt::entity m_Handle = entt::entity(0xFFFFFFFF);

		World* m_ParentWorld = nullptr;

	};
} // namespace Luden

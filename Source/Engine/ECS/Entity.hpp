#pragma once

#include "World.hpp"

namespace Luden
{
	class Entity
	{
		friend class World;
	public:
		Entity() = default;
		Entity(World* pWorld) 
			: m_ParentWorld(pWorld), m_Handle(entt::null)
		{
		}
		Entity(World* pWorld, entt::entity Handle)
			: m_ParentWorld(pWorld), m_Handle(Handle)
		{
		}
		Entity(const Entity& Other)
			: m_ParentWorld(Other.m_ParentWorld), m_Handle(Other.m_Handle)
		{
		}

		bool IsValid();
		bool IsAlive();

		entt::entity& GetHandle()
		{
			return m_Handle;
		}

		template<typename T>
		bool HasComponent()
		{
			return m_ParentWorld->Registry->any_of<T>(m_Handle);
		}

		template<typename T, typename... Args>
		void AddComponent(Args&&... InArgs)
		{
			m_ParentWorld->Registry->emplace<T>(m_Handle, std::forward<Args>(InArgs)...);
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

		bool operator==(const Entity& Other) const
		{
			return (m_Handle == Other.m_Handle) && (m_ParentWorld == Other.m_ParentWorld);
		}

	private:
		entt::entity m_Handle = entt::entity(0xFFFFFFFF);

		World* m_ParentWorld = nullptr;

	};
} // namespace Luden

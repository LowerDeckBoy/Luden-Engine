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
			: m_ParentWorld(pWorld), m_Handle(entt::null_t())
		{
		}
		Entity(World* pWorld, entt::entity Handle)
			: m_ParentWorld(pWorld), m_Handle(Handle)
		{
		}

		bool IsValid();

		template<typename T>
		bool HasComponent()
		{
			return m_ParentWorld->m_Registry->any_of<T>(m_Handle);
		}

		template<typename T, typename... Args>
		void AddComponent(Args&&... InArgs)
		{
			m_ParentWorld->m_Registry->emplace<T>(m_Handle, std::forward<Args>(InArgs)...);
		}

		template<typename T>
		T& GetComponent()
		{
			return m_ParentWorld->m_Registry->get<T>(m_Handle);
		}

		template<typename T>
		void RemoveComponent()
		{
			if (HasComponent<T>())
			{
				m_ParentWorld->m_Registry->remove<T>(m_Handle);
			}
		}

	private:
		entt::entity m_Handle;

		World* m_ParentWorld;

	};
} // namespace Luden


#include "Entity.hpp"

namespace Luden
{
	bool Entity::IsValid()
	{
		return m_ParentWorld->Registry->valid(m_Handle);
	}

	bool Entity::IsAlive()
	{
		return m_ParentWorld != nullptr;
	}
} // namespace Luden

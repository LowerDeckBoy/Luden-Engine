
#include "Entity.hpp"

namespace Luden
{
	bool Entity::IsValid()
	{
		return m_ParentWorld->m_Registry->valid(m_Handle);
	}
} // namespace Luden

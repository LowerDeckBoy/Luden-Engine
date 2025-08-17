#pragma once

#include <string>

namespace Luden::ecs
{
	// Note:
	// It should be treated as *header* component.
	// Should contain core information about entity.
	struct NameComponent
	{
		NameComponent() = default;
		NameComponent(std::string_view Name) : Name(Name) {}

		std::string Name = "";

		bool bVisibleInScene = true;
	};
} // namespace Luden::ecs

#pragma once

#include <string>

namespace Luden::ecs
{
	struct NameComponent
	{
		NameComponent() = default;
		NameComponent(std::string_view Name) : Name(Name) {}

		std::string Name = "";
	};
} // namespace Luden::ecs

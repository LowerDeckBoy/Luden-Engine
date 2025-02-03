#pragma once

#include <string>

namespace Luden::ecs
{
	struct TagComponent
	{
		TagComponent() = default;
		TagComponent(std::string_view Tag) : Name(Tag) {}

		std::string Name = "";
	};
} // namespace Luden::ecs

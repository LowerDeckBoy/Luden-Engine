#pragma once

#include <Core/String.hpp>

namespace Luden
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		void Load();
		void Save();

		std::string Name;

	private:
		//DirectionalLight SunLight;
		//std::vector<PointLight> PointLights;
		//std::vector<SpotLight> SpotLights;
		 
		//std::vector<Model> Models;


	};

} // namespace Luden

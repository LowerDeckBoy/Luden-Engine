#pragma once

/*=================================================================================
	Engine/Scene/SceneSerializer.hpp

	For now scene management is simple JSON loading/saving as given:
	- Array of models:
		- Model name,
		- Path to .gltf / .fbx,
		- Model transformation vectors: Translation, Rotation, Scale,
	- [TODO] Array of lights (and their properties):
		- Directional Light - Sun,
		- Point Lights
=================================================================================*/

#include "Scene.hpp"
#include <Core/File.hpp>

namespace Luden
{
	class AssetImporter;

	class SceneSerializer
	{
	public:
		SceneSerializer() = default;
		~SceneSerializer() = default;

		static void Load(AssetImporter* pImporter, Scene* pScene, Filepath Path);
		
	};
} // namespace Luden

#include "Asset/AssetImporter.hpp"
#include "SceneSerializer.hpp"
#include <Core/Logger.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <print>

namespace Luden
{
	void SceneSerializer::Load(AssetImporter* pImporter, Scene* pScene, Filepath Path)
	{
		if (!File::Exists(Path))
		{
			const auto& filename = Path.filename().string();
			LOG_WARNING("File: {0} doesn't exists!", filename);

			return;
		}

		const auto fileExtension = File::GetExtension(Path);

		if (fileExtension != ".json")
		{
			LOG_WARNING("Currently only json based files can be used as Scene!");

			return;
		}

		std::print("{0}", std::format("\x1b[32m[Debug]\x1b[37m Scene loading: {} info:", (Path.filename().string())));

		std::ifstream file(Path.c_str());
		nlohmann::json json = nlohmann::json::parse(file);
		
		for (const auto& record : json["scene"]["models"])
		{
			const auto& name		= std::string(record["name"]);
			const auto& path		= std::string(record["path"]);
		
			const auto& position	= record["transform"]["position"];
			const auto& rotation	= record["transform"]["rotation"];
			const auto& scale		= record["transform"]["scale"];

			std::print("\n\t- loading: {0}", name);

			Model model{};
			pScene->GetWorld()->CreateEntity(&model);
			
			model.AddComponent<ecs::NameComponent>(name);
			model.AddComponent<ecs::TransformComponent>(
				 DirectX::XMFLOAT3(position[0], position[1], position[2]),
				 DirectX::XMFLOAT3(rotation[0], rotation[1], rotation[2]),
				 DirectX::XMFLOAT3(scale[0], scale[1], scale[2]));

			auto startTime = std::chrono::high_resolution_clock::now();
			pImporter->ImportStaticMesh(path, model);
			auto endTime = std::chrono::high_resolution_clock::now();

			std::print(", load time: {0}", std::chrono::duration<f64>(endTime - startTime));

			model.SetFilepath(path);
			pScene->Models.emplace_back(model);
			
		}

		std::println();

		file.close();
		
	}
} // namespace Luden

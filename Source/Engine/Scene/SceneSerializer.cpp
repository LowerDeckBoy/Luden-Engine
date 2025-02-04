#include "Asset/AssetImporter.hpp"
#include "SceneSerializer.hpp"
#include <Core/Logger.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

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

		std::string sceneInfo = std::format("Scene: {} info:\n", (Path.filename().string()));

		std::ifstream f(Path.c_str());
		nlohmann::json json = nlohmann::json::parse(f);
		
		for (const auto& record : json["scene"]["models"])
		{
			const auto& name = std::string(record["name"]);
			const auto& path = std::string(record["path"]);
		
			Model model{};
			pScene->GetWorld()->CreateEntity(&model);
			
			model.AddComponent<ecs::TagComponent>(name);
			model.AddComponent<ecs::TransformComponent>();

			auto startTime = std::chrono::high_resolution_clock::now();
			pImporter->ImportStaticMesh(path, model.StaticMesh);
			auto endTime = std::chrono::high_resolution_clock::now();

			pScene->Models.emplace_back(model);

			sceneInfo.append(std::format("- {0}, load time: {1}\n", name, std::chrono::duration<f64>(endTime - startTime)));
			
		}

		LOG_DEBUG(sceneInfo);

		f.close();

	}
} // namespace Luden

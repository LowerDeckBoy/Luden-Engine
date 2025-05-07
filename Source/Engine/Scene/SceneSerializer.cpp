#include "Asset/AssetImporter.hpp"
#include "SceneSerializer.hpp"
#include <Core/Logger.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <print>
#include <future>

namespace Luden
{
	bool SceneSerializer::Load(AssetImporter* pImporter, Scene* pScene, Filepath Path)
	{
		if (!File::Exists(Path))
		{
			LOG_WARNING("File: {0} doesn't exists!", Path.filename().string());

			return false;
		}

		if (File::GetExtension(Path) != ".json")
		{
			LOG_WARNING("Currently only json based files can be used as Scene!");

			return false;
		}

		std::print("{0}", std::format("\x1b[32m[Debug]\x1b[37m Scene loading: {} info:", File::GetFilename(Path)));

		std::ifstream file(Path.c_str());
		nlohmann::json json = nlohmann::json::parse(file);

		for (const auto& record : json["scene"]["models"])
		{
			const auto& name = std::string(record["name"]);
			const auto& path = std::string(record["path"]);

			const auto& position = record["transform"]["position"];
			const auto& rotation = record["transform"]["rotation"];
			const auto& scale	 = record["transform"]["scale"];

			std::print("\n\t- loading: {0}", name);

			Model model{};
			// Initialize Enitity with a name, as every model in JSON scene file must have a name anyway.
			pScene->GetWorld()->CreateEntity(&model, name);

			model.AddComponent<ecs::TransformComponent>(
				DirectX::XMFLOAT3(position[0], position[1], position[2]),
				DirectX::XMFLOAT4(rotation[0], rotation[1], rotation[2], rotation[3]),
				DirectX::XMFLOAT3(scale[0], scale[1], scale[2]));

			auto startTime = std::chrono::high_resolution_clock::now();
			if (!pImporter->ImportStaticMesh(path, model))
			{
				LOG_WARNING("Failed to load {}", name);

				//continue;
			}
			auto endTime = std::chrono::high_resolution_clock::now();

			std::print(", load time: {0}", std::chrono::duration<f64>(endTime - startTime));

			model.SetFilepath(path);
			pScene->Models.push_back(std::make_unique<Model>(model));
		}

		std::println();

		file.close();

		// Be default set Scene name as a scene filename.
		pScene->Name = File::GetFilename(Path);

		return true;
	}

	bool SceneSerializer::Save(Scene* /* pScene */, Filepath Path)
	{
		std::ifstream file(Path.c_str());
		nlohmann::json json = nlohmann::json::parse(file);

		return true;
	}
} // namespace Luden

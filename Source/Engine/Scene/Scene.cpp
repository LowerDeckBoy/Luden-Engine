#include "Asset/AssetImporter.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene.hpp"
#include "ECS/Components/LightComponent.hpp"

namespace Luden
{
	Scene::Scene()
		: m_World(new World())
	{
	}

	Scene::Scene(AssetImporter* pAssetImporter)
		: m_World(new World()), m_AssetImporter(pAssetImporter)
	{
		
	}

	Scene::Scene(AssetImporter* pAssetImporter, D3D12Device* pDevice)
		: m_World(new World()), m_AssetImporter(pAssetImporter)
	{
		m_AssetImporter->Device = pDevice;

		SceneDataBuffer = new D3D12ConstantBuffer(pDevice, &SceneData, sizeof(SceneData));

	}

	Scene::~Scene()
	{
		Release();
	}

	void Scene::Build(D3D12Device* pDevice)
	{
		if (IsEmpty())
		{
			LOG_WARNING("Empty scene cannot be built.");

			return;
		}
		
		SceneDataBuffer = new D3D12ConstantBuffer(pDevice, &SceneData, sizeof(SceneData));

		for (auto& model : Models)
		{
			model->Create(pDevice);
		}

	}

	void Scene::Release()
	{
		for (auto& model : Models)
		{
			model->Release();
		}

		if (SceneDataBuffer)
		{
			delete SceneDataBuffer;
			SceneDataBuffer = nullptr;
		}

		if (MaterialBuffer)
		{
			delete MaterialBuffer;
			MaterialBuffer = nullptr;
		}

		if (LightBuffer)
		{
			delete LightBuffer;
			LightBuffer = nullptr;
		}

		Models.clear();

		GetWorld()->Clear();
	}

	void Scene::CreateEntity(Entity& Target)
	{
		m_World->CreateEntity(&Target);
	}

	bool Scene::AddModel(Filepath Path)
	{
		Model model{};

		auto startTime = std::chrono::high_resolution_clock::now();

		if (!m_AssetImporter->ImportStaticMesh(this, Path, model))
		{
			return false;
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		std::println("{0} load time: {1}", File::GetFilename(Path), std::chrono::duration<f64>(endTime - startTime));

		GetWorld()->CreateEntity(&model);

		model.AddComponent<ecs::NameComponent>(File::GetFilename(Path));
		model.AddComponent<ecs::TransformComponent>();
		model.SetFilepath(Path);

		model.Create(m_AssetImporter->Device);

		Models.push_back(std::make_unique<Model>(model));

		// Currently only for debug.
		// Will remove later.
		if (Materials.size() * sizeof(Material) >= MaterialBuffer->GetBufferDesc().Size)
		{
			LOG_ERROR("Scene Material Buffer overload.");
		}
		else
		{
			const usize mapSize = static_cast<usize>(Materials.size() * sizeof(Material));
			std::memcpy(MaterialBuffer->GetBufferDesc().Data, Materials.data(), mapSize);
		}

		return true;
	}

	bool Scene::IsEmpty() const
	{
		return Models.empty();
	}

	void Scene::AddDirectionalLight()
	{
		Entity entity;
		CreateEntity(entity);

		entity.AddComponent<ecs::NameComponent>(std::format("Directional Light {}", DirectionalLights.size()));
		entity.AddComponent<ecs::DirectionalLightComponent>();

		DirectionalLights.push_back(entity);
	}

	void Scene::AddPointLight()
	{
		Entity entity;
		CreateEntity(entity);

		entity.AddComponent<ecs::NameComponent>(std::format("Point Light {}", PointLights.size()));
		
		entity.AddComponent<ecs::PointLightComponent>();

		PointLights.push_back(entity);

	}

	void Scene::UpdateSceneBufferData(SceneCamera* pCamera)
	{
		Consts.Planes[0]				= pCamera->Frustum.Planes[0]; // Right
		Consts.Planes[1]				= pCamera->Frustum.Planes[1]; // Left
		Consts.Planes[2]				= pCamera->Frustum.Planes[2]; // Top
		Consts.Planes[3]				= pCamera->Frustum.Planes[3]; // Bottom
		Consts.Planes[4]				= pCamera->Frustum.Planes[4]; // Far
		Consts.Planes[5]				= pCamera->Frustum.Planes[5]; // Near

		SceneData.View					= pCamera->GetView();
		SceneData.Projection			= pCamera->GetProjection();
		SceneData.InversedView			= pCamera->GetInversedView();
		SceneData.InversedProjection	= pCamera->GetInversedProjection();
		SceneData.InversedViewProjection = DirectX::XMMatrixInverse(nullptr, pCamera->GetViewProjection());
		SceneData.CameraPosition		= pCamera->Position;

		SceneData.Planes[0]				= pCamera->Frustum.Planes[0];
		SceneData.Planes[1]				= pCamera->Frustum.Planes[1];
		SceneData.Planes[2]				= pCamera->Frustum.Planes[2];
		SceneData.Planes[3]				= pCamera->Frustum.Planes[3];
		SceneData.Planes[4]				= pCamera->Frustum.Planes[4];
		SceneData.Planes[5]				= pCamera->Frustum.Planes[5];

		SceneDataBuffer->Update(&SceneData);

		// Works for now. Can't keep it this way, tho.
		std::vector<ecs::PointLightComponent> lights;
		const auto& lightsView = GetRegistry()->view<ecs::PointLightComponent>();
		for (auto [handle, light] : lightsView.each())
		{
			lights.push_back(light);
		}

		const usize mapSize = lights.size() * sizeof(ecs::PointLightComponent);
		std::memcpy(LightBuffer->GetBufferDesc().Data, lights.data(), mapSize);

	}

} // namespace Luden

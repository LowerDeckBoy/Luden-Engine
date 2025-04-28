#include "Asset/AssetImporter.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene.hpp"

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
    }

    Scene::~Scene()
    {
        for (auto& model : Models)
        {
            model->Release();
        }

        if (m_World)
        {
            delete m_World;
        }
    }

    void Scene::Build(D3D12Device* pDevice)
    {
        if (IsEmpty())
        {
            LOG_WARNING("Empty scene cannot be built.");

            return;
        }
        
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

        Models.clear();

        GetRegistry()->clear();
    }

    void Scene::CreateEntity(Entity& Target)
    {
        m_World->CreateEntity(&Target);
    }

    bool Scene::AddModel(Filepath Path)
    {
        Model model{};
        
        if (!m_AssetImporter->ImportStaticMesh(Path, model))
        {
            return false;
        }

        GetWorld()->CreateEntity(&model);

        model.AddComponent<ecs::NameComponent>(File::GetFilename(Path));
        model.AddComponent<ecs::TransformComponent>();
        model.SetFilepath(Path);

        model.Create(m_AssetImporter->Device);

        Models.push_back(std::make_unique<Model>(model));

        return true;
    }

    bool Scene::IsEmpty() const
    {
        return Models.empty();
    }


} // namespace Luden

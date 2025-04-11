#include "Asset/AssetImporter.hpp"
#include "Scene.hpp"

namespace Luden
{
    Scene::Scene()
    {
        m_World = new World();
    }

    Scene::~Scene()
    {
        for (auto& model : Models)
        {
            model.Release();
        }

        if (m_World)
        {
            delete m_World;
        }
    }

    void Scene::Draw()
    {
        if (Models.empty())
        {
            return;
        }
    }

    void Scene::CreateEntity(Entity& Target)
    {
        m_World->CreateEntity(&Target);
    }

    void Scene::AddModel(AssetImporter* pAssetImporter, Filepath Path)
    {
        Model model{};
        GetWorld()->CreateEntity(&model);

        model.AddComponent<ecs::NameComponent>(File::GetFilename(Path));
        model.AddComponent<ecs::TransformComponent>();

        pAssetImporter->ImportStaticMesh(Path, model);

        Models.push_back(model);
    }
} // namespace Luden

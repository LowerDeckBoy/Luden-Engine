#include "Scene.hpp"

namespace Luden
{
    Scene::Scene()
    {
        m_World = new World();
    }

    Scene::~Scene()
    {
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
} // namespace Luden

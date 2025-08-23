#pragma once

#include "ECS/World.hpp"
#include "Graphics/Model.hpp"
#include "SceneCamera.hpp"
#include <Core/File.hpp>
#include <Core/String.hpp>
#include <ECS/Components/LightComponent.hpp>

namespace Luden
{
	class AssetImporter;
	class Renderer;

	struct GlobalConstants
	{
		// World
		// View
		// Projection
		DirectX::XMFLOAT3 Position;
		uint32 pad = 0;
		DirectX::XMFLOAT4 Planes[6];

	};

	struct SceneConstants
	{
		DirectX::XMMATRIX View{};
		DirectX::XMMATRIX Projection{};
		DirectX::XMMATRIX InversedView{};
		DirectX::XMMATRIX InversedProjection{};
		DirectX::XMMATRIX InversedViewProjection{};
		DirectX::XMFLOAT3 CameraPosition{};
		float pad = 0;
		uint32 Width;
		uint32 Height;
		float AspectRatio;
		float pad2 = 0;

		DirectX::XMFLOAT4 Planes[6];

		DirectX::XMFLOAT3 DirectionalPosition;
		float DirectionalIntensity = 5.0f;
		DirectX::XMFLOAT3 DirectionalAmbient;
		float pad3 = 0;
	};

	class Scene
	{
	public:
		Scene();
		Scene(AssetImporter* pAssetImporter);
		Scene(AssetImporter* pAssetImporter, D3D12Device* pDevice);
		~Scene();

		GlobalConstants Consts;

		std::string Name;

		void Update();

		// Initialize Scene and create resources for scene content.
		void Build(D3D12Device* pDevice);

		// Release all the Scene resources.
		// Clear World registry.
		void Release();

		void CreateEntity(Entity& Target);

		bool AddModel(Filepath Path);

		World*			GetWorld()		{ return m_World;			}
		entt::registry* GetRegistry()	{ return m_World->Registry; }

		bool IsEmpty() const;
		
		void AddDirectionalLight();
		void AddPointLight();
		void AddSpotLight();

		std::vector<Entity> DirectionalLights;
		std::vector<Entity> PointLights;
		std::vector<Entity> SpotLights;

		std::vector<std::unique_ptr<Model>> Models;

		D3D12Buffer* TransformsBuffer = nullptr;
		std::vector<ecs::ObjectTransforms> Transforms;
		
		std::vector<Material> Materials;
		D3D12Buffer* MaterialBuffer = nullptr;

		D3D12Buffer* LightBuffer = nullptr;
		D3D12Buffer* SpotLightBuffer = nullptr;

		D3D12ConstantBuffer* SceneDataBuffer = nullptr;
		SceneConstants SceneData{};
		void UpdateSceneBufferData(SceneCamera* pCamera);

		Entity SkyLight;

		AssetImporter* GetAssetImporter() { return m_AssetImporter; }

	private:
		Filepath m_Filepath;

		World* m_World = nullptr;
		AssetImporter* m_AssetImporter = nullptr;

	};
} // namespace Luden

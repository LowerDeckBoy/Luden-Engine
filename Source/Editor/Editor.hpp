#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_win32.h>

#include <Core/Core.hpp>
#include <Core/Timer.hpp>

#include <Engine/Asset/AssetImporter.hpp>
#include <Scene/Scene.hpp>

#include "Panels/PropertyPanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"

namespace Luden
{
	class Renderer;

	class Editor
	{
	public:
		Editor(Platform::Window* pParentWindow, Renderer* pRenderer, Core::Timer* pApplicationTimer);
		~Editor();
		
		void Initialize(Platform::Window* pParentWindow, Renderer* pRenderer, Core::Timer* pApplicationTimer);

		void Begin();
		void End();

	public:

		//
		void SetActiveScene(Scene* pScene);

		// Set image to draw into Scene panel.
		void SetSceneImage(D3D12Descriptor& TextureDescriptor);

	private:
		void DrawEditorLayer();

		void DrawMainMenuBar();

		void DrawSceneControlPanel();

		void DrawSceneImage() const;

		// TODO:
		void DrawPropertyPanel();
		// TODO:
		void DrawLogsPanel();

		// Display debug information about: 
		// - Frames per second, 
		// - Miliseconds taken to render frame, 
		// - Application RAM usage,
		// - GPU's VRAM usage
		// Draws into MainMenuBar panel.
		void DisplayDebugInfo();

		void DrawLightData();

		void DrawEntityComponents(Entity& Entity);

		AssetImporter Importer;

	private:
		Platform::Window* m_ParentWindow;
		Renderer* m_Renderer = nullptr;

		// Scene to display.
		Scene* m_CurrentScene = nullptr;

		ImFont* m_MainFont = nullptr;
		f32 FontSize = 16.0f;

		Core::Timer* m_Timer;

		ImGuiViewport* m_MainViewport;

		ImTextureID m_DrawImageAddress;

		ImGuiStyle* m_Theme;
		
		// Temporarly hard coded paths.
		const char* FontPath		= "..\\..\\Build\\Debug\\Assets\\Fonts\\CascadiaCode.ttf";
		const char* IconsFontPath	= "..\\..\\Build\\Debug\\Assets\\Fonts\\fa-solid-900.ttf";

		static inline int32 DisplayImageIndex = 1;

		Panel::SceneHierarchyPanel	m_HierarchyPanel;
		Panel::PropertyPanel		m_PropertyPanel;

	};
} // namespace Luden

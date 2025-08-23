#pragma once

#include <Scene/Scene.hpp>

#include "Panels/ConfigPanel.hpp"
#include "Panels/ContentBrowserPanel.hpp"
#include "Panels/PropertyPanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include <Core/Core.hpp>
#include <Core/Time/Timer.hpp>
#include <Engine/Asset/AssetImporter.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_win32.h>
#define USE_IMGUI_API
#include <ImGui/ImGuizmo.h>

namespace Luden
{
	class Renderer;
	class D3D12CommandList;
	class D3D12CommandQueue;

	class Editor
	{
	public:
		Editor(Platform::Window* pParentWindow, Renderer* pRenderer, Core::Timer* pApplicationTimer);
		~Editor();
		
		void Initialize(Platform::Window* pParentWindow, Renderer* pRenderer, Core::Timer* pApplicationTimer);

		void Begin();
		void End();
		// Test
		void Render();

	public:
		void SetActiveScene(Scene* pScene);

		// Set image to draw into Scene panel.
		void SetSceneImage(D3D12Descriptor& TextureDescriptor);

	private:
		void DrawEditorLayer();

		void DrawMainMenuBar();

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

		ImGuiStyle* m_Theme;
		
		// Temporarly hard coded paths.
		const char* FontPath		= "..\\..\\Build\\Debug\\Assets\\Fonts\\CascadiaCode.ttf";
		const char* IconsFontPath	= "..\\..\\Build\\Debug\\Assets\\Fonts\\fa-solid-900.ttf";

		Panel::ConfigPanel			m_ConfigPanel;
		Panel::SceneHierarchyPanel	m_HierarchyPanel;
		Panel::PropertyPanel		m_PropertyPanel;
		Panel::ContentBrowserPanel	m_ContentBrowserPanel;

	public:
		// Temporal
		void CreateEditorResources();
		static D3D12Texture* EditorDirectoryTexture;
		static D3D12Texture* EditorFileTexture;

		static D3D12Texture* EditorGLTFTexture;
		static D3D12Texture* EditorGLBTexture;
		static D3D12Texture* EditorFBXTexture;
		static D3D12Texture* EditorOBJTexture;
		static D3D12Texture* EditorPNGTexture;
		static D3D12Texture* EditorJPGTexture;
		static D3D12Texture* EditorJPEGTexture;
		static D3D12Texture* EditorBINTexture;

		// TEST
		D3D12CommandList* m_EditorCommandList;
		D3D12CommandQueue* m_EditorCommandQueue;
		//D3D12DescriptorHeap* m_EditorCommandQueue;

	};
} // namespace Luden

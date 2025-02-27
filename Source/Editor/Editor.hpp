#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_win32.h>

#include <Core/Core.hpp>
#include <Core/Timer.hpp>

#include <Scene/Scene.hpp>

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
		void DrawMainMenuBar();

		void DrawSceneControlPanel();

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

		void DrawActorsData();

		// All lights are gathered into single TreeNode
		// just for readability sake.
		void DrawLightData();

		// Indicates which Entity to display in Properties panel.
		Entity m_SelectedEntity;

	private:
		Platform::Window* m_ParentWindow;
		Renderer* m_Renderer = nullptr;

		Scene* m_CurrentScene = nullptr;

		ImFont* m_MainFont = nullptr;
		f32 FontSize = 15.0f;

		Core::Timer* m_Timer;

		ImGuiViewport* m_MainViewport;

		ImGuiStyle* m_Theme;
		
		// Temporarly hard coded paths.
		const char* FontPath		= "..\\..\\Build\\Debug\\Assets\\Fonts\\CascadiaCode-SemiBold.ttf";
		const char* IconsFontPath	= "..\\..\\Build\\Debug\\Assets\\Fonts\\fa-solid-900.ttf";

	};
} // namespace Luden

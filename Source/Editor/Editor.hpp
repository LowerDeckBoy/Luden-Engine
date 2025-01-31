#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_win32.h>

#include <Core/Timer.hpp>

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
		// ??
		//void End(D3D12CommandList* pCommandList);
		void End();

	public:

		// 
		void SetSceneImage(D3D12Descriptor& TextureDescriptor);

	private:
		void DrawSceneControlPanel();

		// Display fps, ms, RAM and VRAM info.
		void DisplayDebugInfo();

	private:
		Platform::Window* m_ParentWindow;
		Renderer* m_Renderer = nullptr;

		ImFont* m_MainFont = nullptr;
		f32 FontSize = 16.0f;

		Core::Timer* m_Timer;

		ImGuiViewport* m_MainViewport;

		ImGuiStyle* m_Theme;

		const char* FontPath		= "..\\..\\Build\\Debug\\Assets\\Fonts\\CascadiaCode-SemiBold.ttf";
		const char* IconsFontPath	= "..\\..\\Build\\Debug\\Assets\\Fonts\\fa-solid-900.ttf";

	};
} // namespace Luden

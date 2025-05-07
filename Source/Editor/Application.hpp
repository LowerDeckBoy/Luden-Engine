#pragma once

#include <Asset/AssetImporter.hpp>
#include <Core/Timer.hpp>
#include <Engine/RHI/Defines.hpp>
#include <Engine/Scene/Scene.hpp>
#include <Platform/Window.hpp>
#include <Renderer/Renderer.hpp>

#include "Editor.hpp"

namespace Luden
{
	class Application
	{
	public:
		Application();
		~Application();

		void Initialize();
		void Run();
		void Shutdown();

		static AssetImporter Importer;

		Scene* MainScene;

		std::map<std::string, Scene*> Scenes;

		SceneCamera* Camera;

	protected:
		Platform::Window Window;
		std::unique_ptr<Editor> m_Editor;
		
		static LRESULT CALLBACK EditorWindowProc(::HWND Handle, UINT32 Message, WPARAM wParam, LPARAM lParam);

		D3D12RHI* m_D3D12RHI = nullptr;
		Renderer* m_Renderer = nullptr;

		Core::Timer m_Timer;

		static bool bIsResizing;

	};
} // namespace Luden

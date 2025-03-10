#pragma once

#include <Asset/ShaderCompiler.hpp>
#include <Core/Timer.hpp>
#include <Engine/Scene/Scene.hpp>
#include <Engine/Scene/SceneCamera.hpp>
#include <Platform/Window.hpp>
#include <Engine/RHI/Defines.hpp>
#include <Renderer/Renderer.hpp>
#include <Asset/AssetImporter.hpp>

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
		
		static LRESULT  EditorWindowProc(::HWND Handle, UINT32 Message, WPARAM wParam, LPARAM lParam);

		ShaderCompiler* m_ShaderCompiler;

		D3D12RHI* m_D3D12RHI;

		Renderer* m_Renderer;

		Core::Timer m_Timer;

		static bool bIsResizing;

	};
} // namespace Luden

#pragma once

#include <Core/Timer.hpp>
#include <Platform/Window.hpp>



#include <Engine/D3D12/D3D12RHI.hpp>
#include <Asset/ShaderCompiler.hpp>
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

	protected:
		Platform::Window Window;
		std::unique_ptr<Editor> m_Editor;
		
		static LRESULT CALLBACK EditorWindowProc(::HWND Handle, UINT32 Message, WPARAM wParam, LPARAM lParam);

		ShaderCompiler* m_ShaderCompiler;

		D3D12RHI* m_D3D12RHI;

		Renderer* m_Renderer;

		Core::Timer m_Timer;

		static bool bIsResizing;

	};
} // namespace Luden

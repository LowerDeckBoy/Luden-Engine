#include "Application.hpp"

#pragma comment(lib, "dxgi")
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_win32.h>

#include <Core/Logger.hpp>

extern "C"
{
	__declspec(dllexport) extern const UINT  D3D12SDKVersion = 614;
	__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Luden
{
	bool Application::bIsResizing = false;

	Application::Application()
	{
		Initialize();
	}

	Application::~Application()
	{
		Shutdown();
	}

	void Application::Initialize()
	{
		Window.Create({
			.Title = "Luden Engine",
			.Width = 1920,
			.Height = 1080,
			.WindowProc = EditorWindowProc,
			.bMaximize = false
		});

		//m_ShaderCompiler = new ShaderCompiler();

		m_D3D12RHI = new D3D12RHI(&Window);
		m_Renderer = new Renderer(&Window, m_D3D12RHI);

		m_Editor = std::make_unique<Editor>(&Window, m_Renderer, &m_Timer);

	}

	void Application::Run()
	{
		
		m_Timer.Reset();

		while (!Window.bShouldClose)
		{
			// If Window is minimized skip rendering.
			if (Window.IsMinimized())
			{
				continue;
			}

			Window.ProcessMessages();

			if (bIsResizing)
			{
				m_Renderer->Resize();
				bIsResizing = false;
				LOG_DEBUG("Resized to {0}x{1}", Window.Width, Window.Height);
				continue;
			}

			m_Timer.Tick();
			m_Timer.GetFrameStats();

			m_Editor->Begin();
			m_Renderer->Render();
			m_Editor->End();
			m_Renderer->Present(Config::Get().SyncInterval);

		}

		m_Renderer->GetRHI()->Wait();
		m_Renderer->GetRHI()->Flush();
	}

	void Application::Shutdown()
	{
		m_Editor.reset();
		delete m_Renderer;
		delete m_D3D12RHI;

		Window.Shutdown();
	}

	LRESULT CALLBACK Application::EditorWindowProc(::HWND Handle, UINT32 Message, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(Handle, Message, wParam, lParam))
		{
			return true;
		}

		switch (Message)
		{
		case WM_SIZE:
		{
			if (wParam == SIZE_MAXIMIZED)
			{
				bIsResizing = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				bIsResizing = true;
			}
			else
			{
				bIsResizing = false;
			}
			
			return 0;
		}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO mminfo = (LPMINMAXINFO)lParam;

			// Window min dimensions.
			mminfo->ptMinTrackSize.x = 1280;
			mminfo->ptMinTrackSize.y = 800;

			// Window max dimensions are the ones of current monitor display size.
			mminfo->ptMaxTrackSize.x = ::GetSystemMetrics(SM_CXMAXIMIZED);
			mminfo->ptMaxTrackSize.y = ::GetSystemMetrics(SM_CYMAXIMIZED);

			return 0;
		}
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			break;
		}
		case WM_QUIT:
			[[fallthrough]];
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		}

		return DefWindowProcA(Handle, Message, wParam, lParam);
	}

} // namespace Luden

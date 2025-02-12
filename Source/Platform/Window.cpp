#include "Window.hpp"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi")


namespace Luden::Platform
{
	constexpr COLORREF DarkThemeBackground = COLORREF(RGB(32, 32, 32));
	constexpr LPCSTR WindowClassName = "WindowClass";

	bool Window::bCursorVisible = true;

	Window::Window(WindowDesc Desc)
	{
		Create(Desc);
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Create(WindowDesc Desc)
	{
		if (!Instance)
		{
			Instance = ::GetModuleHandleA(nullptr);
		}

		::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

		::WNDCLASSEXA wcex{};
		wcex.cbSize = sizeof(wcex);
		wcex.hInstance = Instance;
		//wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.style = CS_OWNDC;
		wcex.lpszClassName = WindowClassName;
		wcex.hbrBackground = ::CreateSolidBrush(DarkThemeBackground);
		wcex.lpfnWndProc = Desc.WindowProc;

		if (!::RegisterClassExA(&wcex))
		{
			// Log error
			return;
		}

		Width	= Desc.Width;
		Height	= Desc.Height;

		::RECT windowRect = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
		::AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

		Width = static_cast<uint32_t>(windowRect.right - windowRect.left);
		Height = static_cast<uint32_t>(windowRect.bottom - windowRect.top);

		Handle = ::CreateWindowExA(WS_EX_OVERLAPPEDWINDOW,
			wcex.lpszClassName, Desc.Title,
			WS_OVERLAPPEDWINDOW,
			0, 0,
			Width, Height,
			nullptr,
			nullptr,
			Instance,
			nullptr);

		const int32_t xPos = (::GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		const int32_t yPos = (::GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;

		::SetWindowPos(Handle, nullptr, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

		BOOL bDarkMode = TRUE;
		::DwmSetWindowAttribute(Handle, DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));

		::COLORREF captionColor = DarkThemeBackground;
		::DwmSetWindowAttribute(Handle, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));

		::ShowWindow(Handle, (Desc.bMaximize) ? SW_SHOWMAXIMIZED : SW_SHOW);
		::SetForegroundWindow(Handle);
		::SetFocus(Handle);
		::UpdateWindow(Handle);
	}

	void Window::Shutdown()
	{
		if (Instance)
		{
			::UnregisterClassA(WindowClassName, Instance);

			Instance = nullptr;
			Handle = nullptr;
		}
	}

	void Window::Resize()
	{
		::RECT windowRect{};
		::GetClientRect(Handle, &windowRect);

		::RECT out(0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
		Width	= static_cast<uint32_t>(windowRect.right - windowRect.left);
		Height	= static_cast<uint32_t>(windowRect.bottom - windowRect.top);
	}

	void Window::ProcessMessages()
	{
		MSG msg{};

		if (::PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageA(&msg);	
		}
		
		if (msg.message == WM_QUIT)
		{
			bShouldClose = true;
		}
		
	}

	bool Window::IsMinimized() const
	{
		return ::IsIconic(Handle);
	}

	bool Window::IsMaximized() const
	{
		return ::IsZoomed(Handle);
	}

	void Window::OnCursorShow()
	{
		while (::ShowCursor(true) < 0)
		{
			bCursorVisible = true;
		}
	}

	void Window::OnCursorHide()
	{
		while (::ShowCursor(false) >= 0)
		{
			bCursorVisible = false;
		}
	}

	void Window::SetWindowTitle(std::string_view Title) const
	{
		if (!Title.empty())
		{
			::SetWindowTextA(Handle, Title.data());
		}
	}

	LRESULT Window::DefaultWindowProc(::HWND Handle, UINT32 Message, WPARAM wParam, LPARAM lParam)
	{
		switch (Message)
		{
		case WM_QUIT:
			[[fallthrough]];
		case WM_DESTROY:
			::PostQuitMessage(0);
			break;
		}

		return ::DefWindowProcA(Handle, Message, wParam, lParam);
	}
} // namespace Luden::Platform

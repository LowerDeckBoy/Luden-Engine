#pragma once

#include "Export.hpp"
#include "Platform.hpp"
#include <cstdint>
#include <string>

namespace Luden::Platform
{
	struct PLATFORM_API WindowDesc
	{
		const char*			Title;
		uint32_t			Width;
		uint32_t			Height;
		::WNDPROC const&	WindowProc;

		bool				bMaximize  = false;
	};

	class PLATFORM_API Window
	{
	public:
		Window() = default;
		explicit Window(WindowDesc Desc);
		~Window();

		void Create(WindowDesc Desc);

		void Shutdown();

		void Resize();

		void ProcessMessages();

		bool IsMinimized() const;
		bool IsMaximized() const;

		void OnCursorShow();
		void OnCursorHide();

		void SetWindowTitle(std::string_view Title) const;

		static bool bCursorVisible;

		uint32_t Width	= 1920;
		uint32_t Height	= 1080;

		::HINSTANCE Instance = nullptr;
		::HWND Handle = nullptr;

		static LRESULT CALLBACK DefaultWindowProc(::HWND Handle, UINT32 Message, WPARAM wParam, LPARAM lParam);

		::WNDPROC WindowProcedures = DefWindowProcA;

		bool bShouldClose = false;

	};
} // namespace Luden::Platform

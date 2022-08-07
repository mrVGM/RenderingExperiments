#include "window.h"

namespace _windowData
{
	static bool m_classRegistered = false;
	const wchar_t* m_className = L"MyWindow";
}

void rendering::Window::RegisterWindowClass()
{
	if (!_windowData::m_classRegistered)
	{
		WNDCLASSEXW wcex;

		ZeroMemory(&wcex, sizeof(wcex));
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = &StaticWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandle(NULL);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = _windowData::m_className;

		RegisterClassExW(&wcex);

		_windowData::m_classRegistered = true;
	}
}

LRESULT rendering::Window::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE)
	{
		LPCREATESTRUCT data = (LPCREATESTRUCT)lParam;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data->lpCreateParams);
		auto* window = (Window*)data->lpCreateParams;
		window->m_hwnd = hWnd;
	}

	// Process messages by window message function
	Window* window = (Window*) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (window)
	{
		return window->WndProc(uMsg, wParam, lParam);
	}
	else
	{
		return static_cast<LRESULT>(DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
}

LRESULT rendering::Window::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		m_hwnd = NULL;
		break;
	}

	case WM_KEYDOWN:
	{
		return 0;
	}

	case WM_KEYUP:
	{
		return 0;
	}
	}

	return static_cast<LRESULT>(DefWindowProc(m_hwnd, uMsg, wParam, lParam));
}

rendering::Window::Window()
{
	RegisterWindowClass();
	Create();
}

rendering::Window::~Window()
{
	if (m_hwnd != NULL) {
		DestroyWindow(m_hwnd);
		m_hwnd = nullptr;
	}
}

void rendering::Window::Create()
{
	DWORD dwStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	DWORD dxExStyle = 0;

	RECT windowRect;
	windowRect.left = 50;
	windowRect.top = 50;
	windowRect.right = windowRect.left + 1600;
	windowRect.bottom = windowRect.top + 900;

	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	CreateWindowW(
		_windowData::m_className,
		L"My Window",
		dwStyle,
		windowRect.left, windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, GetModuleHandle(NULL), this);
}

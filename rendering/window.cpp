#include "window.h"

#include "nativeFunc.h"

namespace _windowData
{
	static bool m_classRegistered = false;
	const wchar_t* m_className = L"MyWindow";
	bool m_created = false;
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
		Destroy();
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
}

rendering::Window::~Window()
{
	Destroy();
}

interpreter::Value rendering::Window::Create()
{
	Window* wnd = new Window();
	interpreter::Value res(*wnd);

	wnd->RegisterProperty("create", &wnd->m_create);
	wnd->RegisterProperty("drag", &wnd->m_drag);

	interpreter::Value create = interpreter::CreateNativeMethod(*wnd, 2, [](interpreter::Value scope) {
		interpreter::Value self = scope.GetProperty("self");
		Window* wnd = static_cast<Window*>(self.GetManagedValue());
		interpreter::Value width = scope.GetProperty("param0");
		interpreter::Value height = scope.GetProperty("param1");
		wnd->Create(width.GetNum(), height.GetNum());
		return interpreter::Value();
	});

	interpreter::Value drag = interpreter::CreateNativeMethod(*wnd, 0, [](interpreter::Value scope) {
		interpreter::Value self = scope.GetProperty("self");
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, NULL, 0, 0)) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return interpreter::Value();
	});

	wnd->SetProperty("create", create);
	wnd->SetProperty("drag", drag);

	return res;
}

void rendering::Window::Create(int width, int height)
{
	if (_windowData::m_created) {
		return;
	}
	_windowData::m_created = true;

	DWORD dwStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	DWORD dxExStyle = 0;

	RECT windowRect;
	windowRect.left = 50;
	windowRect.top = 50;
	windowRect.right = windowRect.left + width;
	windowRect.bottom = windowRect.top + height;

	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	CreateWindowW(
		_windowData::m_className,
		L"Render Window",
		dwStyle,
		windowRect.left, windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, GetModuleHandle(NULL), this);
}

void rendering::Window::Destroy()
{
	if (m_hwnd != NULL) {
		DestroyWindow(m_hwnd);
		m_hwnd = nullptr;
	}
}

#include "window.h"

#include "nativeFunc.h"

namespace
{
	static bool m_classRegistered = false;
	const wchar_t* m_className = L"MyWindow";
	bool m_created = false;
}

void rendering::Window::RegisterWindowClass()
{
	if (!m_classRegistered)
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
		wcex.lpszClassName = m_className;

		RegisterClassExW(&wcex);

		m_classRegistered = true;
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

void rendering::Window::InitProperties(interpreter::NativeObject& nativeObject)
{
	interpreter::Value create = interpreter::CreateNativeMethod(nativeObject, 2, [](interpreter::Value scope) {
		interpreter::Value self = scope.GetProperty("self");
		interpreter::NativeObject* obj = static_cast<interpreter::NativeObject*>(self.GetManagedValue());

		Window& wnd = static_cast<Window&>(obj->GetNativeObject());

		interpreter::Value width = scope.GetProperty("param0");
		interpreter::Value height = scope.GetProperty("param1");


		if (width.GetType() != interpreter::ScriptingValueType::Number || width.GetNum() < 0) {
			scope.SetProperty("exception", interpreter::Value("Bad width value!"));
			return interpreter::Value();
		}

		if (height.GetType() != interpreter::ScriptingValueType::Number || height.GetNum() < 0) {
			scope.SetProperty("exception", interpreter::Value("Bad height value!"));
			return interpreter::Value();
		}

		wnd.m_width = static_cast<UINT>(width.GetNum());
		wnd.m_height = static_cast<UINT>(height.GetNum());

		wnd.Create(width.GetNum(), height.GetNum());
		return interpreter::Value();
	});

	interpreter::Value& widthVal = GetOrCreateProperty(nativeObject, "width");
	interpreter::Value& heightVal = GetOrCreateProperty(nativeObject, "height");

	widthVal = interpreter::CreateNativeMethod(nativeObject, 0, [](interpreter::Value scope) {
		interpreter::Value self = scope.GetProperty("self");
		Window* wnd = static_cast<Window*>(interpreter::NativeObject::ExtractNativeObject(self));

		return interpreter::Value(wnd->m_width);
	});

	heightVal = interpreter::CreateNativeMethod(nativeObject, 0, [](interpreter::Value scope) {
		interpreter::Value self = scope.GetProperty("self");
		Window* wnd = static_cast<Window*>(interpreter::NativeObject::ExtractNativeObject(self));

		return interpreter::Value(wnd->m_height);
	});

	interpreter::Value drag = interpreter::CreateNativeMethod(nativeObject, 0, [](interpreter::Value scope) {
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

	interpreter::Value alive = interpreter::CreateNativeMethod(nativeObject, 0, [](interpreter::Value scope) {
		interpreter::Value self = scope.GetProperty("self");
		interpreter::NativeObject* obj = static_cast<interpreter::NativeObject*>(self.GetManagedValue());

		Window& wnd = static_cast<Window&>(obj->GetNativeObject());

		return interpreter::Value((bool)wnd.m_hwnd);
	});
	
	interpreter::Value& createProp = GetOrCreateProperty(nativeObject, "create");
	interpreter::Value& dragProp = GetOrCreateProperty(nativeObject, "drag");
	interpreter::Value& aliveProp = GetOrCreateProperty(nativeObject, "isAlive");

	createProp = create;
	dragProp = drag;
	aliveProp = alive;
}


void rendering::Window::Create(int width, int height)
{
	if (m_created) {
		return;
	}
	m_created = true;

	DWORD dwStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	DWORD dxExStyle = 0;

	RECT windowRect;
	windowRect.left = 50;
	windowRect.top = 50;
	windowRect.right = windowRect.left + width;
	windowRect.bottom = windowRect.top + height;

	AdjustWindowRect(&windowRect, dwStyle, FALSE);

	CreateWindow(
		m_className,
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

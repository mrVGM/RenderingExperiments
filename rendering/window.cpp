#include "window.h"

#include "nativeFunc.h"
#include "utils.h"
#include "api.h"
#include "helper/inputHandler.h"

#include "dxRenderer.h"

#include <thread>
#include <chrono>
#include <hidusage.h>

namespace
{
	static bool m_classRegistered = false;
	const wchar_t* m_className = L"MyWindow";
	bool m_created = false;

	rendering::Window* m_wnd = nullptr;
	bool m_windowLoopRunning = true;
	std::thread* m_windowLoopThread = nullptr;

	std::chrono::system_clock::time_point m_lastTick;

	interpreter::Value GetRenderFunc()
	{
		using namespace interpreter;

		Value api = rendering::GetAPI();
		Value app_context = api.GetProperty("app_context");

		return app_context.GetProperty("render");
	}

	rendering::DXRenderer* GetRenderer()
	{
		using namespace interpreter;

		Value api = rendering::GetAPI();
		Value app_context = api.GetProperty("app_context");

		Value renderer = app_context.GetProperty("renderer");

		if (renderer.IsNone()) {
			return nullptr;
		}

		return static_cast<rendering::DXRenderer*>(NativeObject::ExtractNativeObject(renderer));
	}

	rendering::InputHandler* GetInputHandler()
	{
		using namespace interpreter;

		Value api = rendering::GetAPI();
		Value app_context = api.GetProperty("app_context");

		Value inputHandlerValue =  app_context.GetProperty("inputHandler");
		if (inputHandlerValue.IsNone()) {
			return nullptr;
		}

		return dynamic_cast<rendering::InputHandler*>(NativeObject::ExtractNativeObject(inputHandlerValue));
	}

	void WindowLoop()
	{
		rendering::Window* wnd = m_wnd;
		m_wnd = nullptr;

		wnd->Create();

		m_lastTick = std::chrono::system_clock::now();

		while (m_windowLoopRunning) {
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			auto nowNN = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
			auto lastTickNN = std::chrono::time_point_cast<std::chrono::nanoseconds>(m_lastTick);
			long long deltaNN = nowNN.time_since_epoch().count() - lastTickNN.time_since_epoch().count();
			double dt = deltaNN / 1000000000.0;
			m_lastTick = now;
			
			rendering::DXRenderer* renderer = GetRenderer();
			if (!renderer) {
				continue;
			}

			std::string error;
			bool res = renderer->Render(error);
			if (!res) {
				return;
			}
			
			wnd->WindowTick(dt);

			res = renderer->Wait(error);
			if (!res) {
				return;
			}
		}
	}
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
		window->RegisterRawInputDevice();
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
		m_inputInfo.m_keysDown.insert(wParam);
		return 0;
	}

	case WM_KEYUP:
	{
		m_inputInfo.m_keysDown.erase(wParam);
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		m_inputInfo.m_leftMouseButtonDown = true;
		return 0;
	}
	case WM_LBUTTONUP:
	{
		m_inputInfo.m_leftMouseButtonDown = false;
		return 0;
	}

	case WM_RBUTTONDOWN:
	{
		m_inputInfo.m_rightMouseButtonDown = true;
		return 0;
	}
	case WM_RBUTTONUP:
	{
		m_inputInfo.m_rightMouseButtonDown = false;
		return 0;
	}

	case WM_INPUT:
	{
		UINT dwSize = sizeof(RAWINPUT);
		static BYTE lpb[sizeof(RAWINPUT)];
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)lpb;
		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE) {
				m_inputInfo.m_mouseMovement[0] += raw->data.mouse.lLastX;
				m_inputInfo.m_mouseMovement[1] += raw->data.mouse.lLastY;
			}
			else if (raw->data.mouse.usFlags == MOUSE_MOVE_ABSOLUTE) {
				m_inputInfo.m_mouseMovement[0] = raw->data.mouse.lLastX;
				m_inputInfo.m_mouseMovement[1] = raw->data.mouse.lLastY;
			}
		}
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

	if (m_windowLoopThread) {
		m_windowLoopRunning = false;
		m_windowLoopThread->join();
		delete m_windowLoopThread;
		m_windowLoopThread = nullptr;
	}
}

void rendering::Window::WindowTick(double dt)
{
	using namespace interpreter;
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (!GetMessage(&msg, NULL, 0, 0)) {
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	InputHandler* inputHandler = GetInputHandler();
	if (!inputHandler) {
		return;
	}

	m_inputInfo.m_source = this;
	inputHandler->HandleInput(dt, m_inputInfo);
}

void rendering::Window::InitProperties(interpreter::NativeObject& nativeObject)
{
#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	using namespace interpreter;

	interpreter::Value create = interpreter::CreateNativeMethod(nativeObject, 2, [](interpreter::Value scope) {
		if (m_created) {
			THROW_EXCEPTION("Window already created!")
		}

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

		if (m_windowLoopThread) {
			m_windowLoopRunning = false;
			m_windowLoopThread->join();
			delete m_windowLoopThread;
			m_windowLoopThread = nullptr;
		}

		m_wnd = &wnd;
		m_windowLoopRunning = true;
		m_windowLoopThread = new std::thread(WindowLoop);
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

	interpreter::Value alive = interpreter::CreateNativeMethod(nativeObject, 0, [](interpreter::Value scope) {
		interpreter::Value self = scope.GetProperty("self");
		interpreter::NativeObject* obj = static_cast<interpreter::NativeObject*>(self.GetManagedValue());

		Window& wnd = static_cast<Window&>(obj->GetNativeObject());

		return interpreter::Value((bool)wnd.m_hwnd);
	});
	
	interpreter::Value& createProp = GetOrCreateProperty(nativeObject, "create");
	interpreter::Value& aliveProp = GetOrCreateProperty(nativeObject, "isAlive");

	createProp = create;
	aliveProp = alive;

#undef THROW_EXCEPTION
}

void rendering::Window::RegisterRawInputDevice()
{
	RAWINPUTDEVICE rid[1];
	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[0].dwFlags = RIDEV_INPUTSINK;
	rid[0].hwndTarget = m_hwnd;
	RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
}


void rendering::Window::Create()
{
	m_created = true;

	DWORD dwStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	DWORD dxExStyle = 0;

	RECT windowRect;
	windowRect.left = 50;
	windowRect.top = 50;
	windowRect.right = windowRect.left + m_width;
	windowRect.bottom = windowRect.top + m_height;

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
		m_windowLoopRunning = false;

		DestroyWindow(m_hwnd);
		m_hwnd = nullptr;
	}
}

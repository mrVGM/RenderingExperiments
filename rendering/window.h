#pragma once

#include "nativeObject.h"

#include <Windows.h>
#include <set>


namespace rendering
{
	class Window;
	struct InputInfo
	{
		Window* m_source = nullptr;
		std::set<WPARAM> m_keysDown;
		bool m_leftMouseButtonDown = false;
		bool m_rightMouseButtonDown = false;
		long m_mouseMovement[2];
	};

	class Window : public interpreter::INativeObject
	{
		InputInfo m_inputInfo;

		void RegisterWindowClass();
		void Destroy();

		static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		void RegisterRawInputDevice();

	public:
		HWND m_hwnd;
		UINT m_width;
		UINT m_height;
		Window();
		~Window();

		void Create();
		void WindowTick(double dt);
	};
}
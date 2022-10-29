#pragma once

#include "nativeObject.h"

#include <Windows.h>
#include <list>


namespace rendering
{
	class Window : public interpreter::INativeObject
	{
		std::list<WPARAM> m_keysDown;
		std::list<WPARAM> m_keysUp;

		void RegisterWindowClass();
		void Create();
		void Destroy();

		static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
		HWND m_hwnd;
		UINT m_width;
		UINT m_height;
		Window();
		~Window();
	};
}
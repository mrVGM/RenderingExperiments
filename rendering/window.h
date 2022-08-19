#include "nativeObject.h"

#include <Windows.h>


namespace rendering
{
	class Window : public interpreter::INativeObject
	{
		void RegisterWindowClass();
		void Create(int width, int height);
		void Destroy();

		static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
		HWND m_hwnd;
		Window();
		~Window();
	};
}
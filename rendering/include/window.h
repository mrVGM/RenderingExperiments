#include "renderObject.h"
#include "value.h"

#include <Windows.h>


namespace rendering
{
	class Window : public RenderObject
	{
		void RegisterWindowClass();
		void Create(int width, int height);
		void Destroy();

		HWND m_hwnd;

		static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
		Window();
	public:
		static interpreter::Value Create();
		~Window();
		interpreter::Value m_create;
		interpreter::Value m_drag;
	};
}
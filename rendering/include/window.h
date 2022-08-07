#include <Windows.h>

namespace rendering
{
	class Window
	{
		void RegisterWindowClass();
		void Create();

		HWND m_hwnd;

		static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	public:
		Window();
		~Window();
	};
}
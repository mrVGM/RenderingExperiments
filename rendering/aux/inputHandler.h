#pragma once

#include <Windows.h>
#include <list>

namespace rendering
{
	class InputHandler
	{
	public:
		virtual void HandleInput(double dt, std::list<WPARAM>& keysDown, std::list<WPARAM>& keysUp) = 0;
	};
}
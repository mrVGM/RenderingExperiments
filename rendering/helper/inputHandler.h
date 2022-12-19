#pragma once

#include <Windows.h>
#include <list>

namespace rendering
{
	struct InputInfo;
	class InputHandler
	{
	public:
		virtual void HandleInput(double dt, const InputInfo& inputInfo) = 0;
		virtual void RunUpdaters(double dt) = 0;
	};
}

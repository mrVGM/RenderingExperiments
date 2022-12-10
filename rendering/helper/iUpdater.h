#pragma once

#include <Windows.h>
#include <list>

namespace rendering::helper
{
	class IUpdater
	{
	public:
		virtual void Update(double dt) = 0;
	};
}

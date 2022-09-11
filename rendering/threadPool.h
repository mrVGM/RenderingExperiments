#pragma once

namespace rendering::threadPool
{
	struct Runnable
	{
		virtual void Run() = 0;
		virtual void Ready() = 0;
	};
	void StartRoutine(Runnable* runnable);
}
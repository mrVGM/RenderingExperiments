#pragma once

#include "nativeObject.h"

#include "dxFence.h"

#include <d3d12.h>
#include <string>
#include <thread>

namespace rendering
{
	class DXFenceEvent : public interpreter::INativeObject
	{
		int m_id = -1;
		std::thread* m_waitThread = nullptr;

		// TODO: REMOVE this value
		interpreter::Value m_callback;

		HANDLE m_fenceEvent = nullptr;

		void InitProperties(interpreter::NativeObject& nativeObject);
		bool Create(std::string& errorMessage);

		bool AttachToFence(DXFence& fence, std::string errorMessage);
	public:
		void WaitBlocking();
		~DXFenceEvent();
	};
}
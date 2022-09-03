#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <wrl.h>

#include <mutex>

namespace rendering
{
	class DXFence : public interpreter::INativeObject
	{
		int m_eventCounter = 0;
		std::mutex m_eventCounterMutex;

		void InitProperties(interpreter::NativeObject& nativeObject) override;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	public:
		ID3D12Fence* GetFence() const;
	};
}
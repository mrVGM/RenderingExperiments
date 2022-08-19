#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <wrl.h>

namespace rendering
{
	class DXFence : public interpreter::INativeObject
	{
		void InitProperties(interpreter::NativeObject& nativeObject) override;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	public:
		ID3D12Fence* GetFence() const;
	};
}
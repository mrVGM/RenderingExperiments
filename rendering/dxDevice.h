#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <string>

#include "d3dx12.h"
#include "nativeObject.h"
#include "dxBuffer.h"

namespace rendering
{
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

	class DXDevice : public interpreter::INativeObject
	{
        Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;

        Microsoft::WRL::ComPtr<ID3D12Device> m_device;

        bool Create(std::string& errorMessage);

        void InitProperties(interpreter::NativeObject& nativeObject) override;
    public:

        ID3D12Device& GetDevice();
        IDXGIFactory4* GetFactory() const;
	};

}
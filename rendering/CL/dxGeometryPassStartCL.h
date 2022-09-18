#pragma once

#include "nativeObject.h"

#include "d3dx12.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXGeometryPassStartCL : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_diffuseTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_specularTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_positionTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_normalTex;

		UINT m_rtvDescriptorSize;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			UINT texturesWidth,
			UINT texturesHeight,
			std::string& errorMessage);

		bool Execute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);

	public:
	};
}
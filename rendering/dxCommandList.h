#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXCommandList : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(ID3D12Device* device, ID3DBlob* vertexShader, ID3DBlob* pixelShader, std::string& errorMessage);
	public:
	};
}
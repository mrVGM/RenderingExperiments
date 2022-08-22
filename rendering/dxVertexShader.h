#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "nativeObject.h"

namespace rendering
{
	class DXVertexShader : public interpreter::INativeObject
	{
		void InitProperties(interpreter::NativeObject& nativeObject) override;
		bool Init(const std::string& shaderCode, std::string& errorMessage);

		Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShader;
	public:
		ID3DBlob* GetCompiledShader() const;
	};
}
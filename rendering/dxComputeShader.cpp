#include "dxComputeShader.h"
#include "nativeFunc.h"

#include <d3dcompiler.h>

void rendering::DXComputeShader::InitProperties(interpreter::NativeObject& nativeObject)
{
    using namespace interpreter;

    Value& compile = GetOrCreateProperty(nativeObject, "compile");

    compile = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value self = scope.GetProperty("self");
        NativeObject* selfNativeObject = static_cast<NativeObject*>(self.GetManagedValue());
        DXComputeShader& shader = static_cast<DXComputeShader&>(selfNativeObject->GetNativeObject());

        Value shaderCodeValue = scope.GetProperty("param0");
        if (shaderCodeValue.GetType() != ScriptingValueType::String) {
            scope.SetProperty("exception", Value("Please supply the code of the shader!"));
            return Value();
        }

        std::string shaderCode = shaderCodeValue.GetString();

        std::string error;
        bool res = shader.Init(shaderCode, error);
        if (!res) {
            scope.SetProperty("exception", Value(error));
            return Value();
        }

        return Value();
        });
}

bool rendering::DXComputeShader::Init(const std::string& shaderCode, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

#if DEBUG
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> error;

    HRESULT hRes = D3DCompile(
        shaderCode.c_str(),
        shaderCode.size(),
        NULL,
        nullptr,
        nullptr,
        "CSMain",
        "cs_5_0",
        compileFlags,
        0,
        &m_computeShader,
        &error);

    if (FAILED(hRes)) {
        std::string mess = "Can't compile shader!";
        if (error) {
            char* compilationError = static_cast<char*>(error->GetBufferPointer());
            mess += " - ";
            mess += compilationError;
        }
        errorMessage = mess;
        return false;
    }

    return true;
}

ID3DBlob* rendering::DXComputeShader::GetCompiledShader() const
{
    return m_computeShader.Get();
}

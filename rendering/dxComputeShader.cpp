#include "dxComputeShader.h"
#include "nativeFunc.h"
#include "api.h"

#include <d3dcompiler.h>

void rendering::DXComputeShader::InitProperties(interpreter::NativeObject& nativeObject)
{
    using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

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

    Value& loadPrecompiled = GetOrCreateProperty(nativeObject, "loadPrecompiled");
    loadPrecompiled = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXComputeShader* self = static_cast<DXComputeShader*>(NativeObject::ExtractNativeObject(selfValue));

        Value shaderNameValue = scope.GetProperty("param0");
        if (shaderNameValue.GetType() != ScriptingValueType::String) {
            THROW_EXCEPTION("Please supply shader name!")
        }

        std::string error;
        bool res = self->LoadPrecompiledShader(shaderNameValue.GetString(), error);

        if (!res) {
            THROW_EXCEPTION(error)
        }
        return Value();
    });


#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
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

bool rendering::DXComputeShader::LoadPrecompiledShader(const std::string& name, std::string& errorMessage)
{
    using namespace interpreter;
    Value api = GetAPI();
    Value appContext = api.GetProperty("app_context");

    std::string rootDir = appContext.GetProperty("root_dir").GetString() + "shaders\\";

    std::string preCompiledName = rootDir + "cs_" + name.substr(0, name.size() - 5) + ".fxc";
    std::wstring preCompiledNameW(preCompiledName.begin(), preCompiledName.end());

    THROW_ERROR(
        D3DReadFileToBlob(preCompiledNameW.c_str(), &m_computeShader),
        "Can't load the precompiled Compute Shader!"
    )

    return true;
}

ID3DBlob* rendering::DXComputeShader::GetCompiledShader() const
{
    return m_computeShader.Get();
}

#undef THROW_ERROR
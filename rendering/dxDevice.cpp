#include "dxDevice.h"

#include "nativeFunc.h"
#include "window.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxFence.h"
#include "dxCommandQueue.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}


bool rendering::DXDevice::Create(std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    UINT dxgiFactoryFlags = 0;

#if DEBUG
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    THROW_ERROR(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)), "Can't create DXGIFactoty!")

    {
        THROW_ERROR(D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        ), "Can't Create device");
    }

    return true;
}

void rendering::DXDevice::UpdateCurrentFrameIndex()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

bool rendering::DXDevice::Present(std::string& errorMessage)
{    
    THROW_ERROR(m_swapChain->Present(1, 0),
        "Can't present Swap Chain!")

    return true;
}

#undef THROW_ERROR


void rendering::DXDevice::InitProperties(interpreter::NativeObject& nativeObject)
{
	using namespace interpreter;


#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& updateCurrentFrameIndex = GetOrCreateProperty(nativeObject, "updateCurrentFrameIndex");
    updateCurrentFrameIndex = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value self = scope.GetProperty("self");
        DXDevice& device = static_cast<DXDevice&>(*NativeObject::ExtractNativeObject(self));

        device.UpdateCurrentFrameIndex();
        return Value();
    });

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value self = scope.GetProperty("self");
        DXDevice& device = static_cast<DXDevice&>(*NativeObject::ExtractNativeObject(self));

        std::string error;
        bool res = device.Create(error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& present = GetOrCreateProperty(nativeObject, "present");
    present = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value self = scope.GetProperty("self");
        NativeObject* selfContainer = static_cast<NativeObject*>(self.GetManagedValue());
        DXDevice& device = static_cast<DXDevice&>(selfContainer->GetNativeObject());

        std::string errorMessage;
        bool res = device.Present(errorMessage);

        if (!res) {
            THROW_EXCEPTION(errorMessage)
        }

        return Value();
    });

#undef THROW_EXCEPTION
}

ID3D12Device& rendering::DXDevice::GetDevice()
{
    return *m_device.Get();
}

IDXGIFactory4* rendering::DXDevice::GetFactory() const
{
    return m_factory.Get();
}

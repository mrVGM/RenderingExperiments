#include "renderstage/dxClearDSRS.h"

#include "nativeFunc.h"

#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxSwapChain.h"
#include "dxBuffer.h"
#include "dxDescriptorHeap.h"
#include "dxCommandQueue.h"
#include "dxFence.h"

void rendering::renderstage::DXClearDSRS::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

bool rendering::renderstage::DXClearDSRS::Init(DXRenderer& renderer, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    THROW_ERROR(
        renderer.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        renderer.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)),
        "Can't create Command List!")

    m_commandList->ClearDepthStencilView(renderer.GetDSHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close command List!")

    return true;
}

bool rendering::renderstage::DXClearDSRS::Execute(DXRenderer& renderer, std::string& errorMessage)
{
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    renderer.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    return true;
}

#undef THROW_ERROR
#include "dxFenceEvent.h"

#include "dxFence.h"
#include "nativeFunc.h"
#include "utils.h"
#include "value.h"

#include <semaphore>

namespace
{
    std::binary_semaphore m_semaphore{1};
    rendering::DXFenceEvent* m_fenceEventCache = nullptr;

    void waitThread()
    {
        rendering::DXFenceEvent* fenceEvent = m_fenceEventCache;
        m_fenceEventCache = nullptr;
        m_semaphore.release();

        fenceEvent->WaitBlocking();
    }
}

void rendering::DXFenceEvent::InitProperties(interpreter::NativeObject& nativeObject)
{
    using namespace interpreter;

#define THROW_EXCEPTION(message)\
scope.SetProperty("exception", Value(message));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value self = scope.GetProperty("self");
        DXFenceEvent* fenceEvent = static_cast<DXFenceEvent*>(NativeObject::ExtractNativeObject(self));

        std::string error; 
        bool res = fenceEvent->Create(error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& wait = GetOrCreateProperty(nativeObject, "wait");
    wait = CreateNativeMethod(nativeObject, 2, [&](Value scope) {
        Value self = scope.GetProperty("self");
        DXFenceEvent* fenceEvent = static_cast<DXFenceEvent*>(NativeObject::ExtractNativeObject(self));

        Value fenceValue = scope.GetProperty("param0");
        DXFence* fence = dynamic_cast<DXFence*>(NativeObject::ExtractNativeObject(fenceValue));
        if (!fence) {
            THROW_EXCEPTION("Please supply a Fence!");
        }

        Value func = scope.GetProperty("param1");
        if (func.GetType() != ScriptingValueType::Object) {
            THROW_EXCEPTION("Please supply a Callback!");
        }

        fenceEvent->m_id = fence->GetEventID();
        fenceEvent->m_callback = func;

        std::string error;
        bool res = fenceEvent->AttachToFence(*fence, error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        m_semaphore.acquire();
        m_fenceEventCache = fenceEvent;

        fenceEvent->m_waitThread = new std::thread(waitThread);

        return Value();
    });

    Value& getID = GetOrCreateProperty(nativeObject, "getID");
    getID = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value self = scope.GetProperty("self");
        DXFenceEvent* fenceEvent = static_cast<DXFenceEvent*>(NativeObject::ExtractNativeObject(self));

        return Value(fenceEvent->m_id);
    });

#undef THROW_EXCEPTION
}

bool rendering::DXFenceEvent::Create(std::string& errorMessage)
{
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr) {
        errorMessage = "Can't create Fence Event!";
        return false;
    }

    return true;
}

bool rendering::DXFenceEvent::AttachToFence(DXFence& fence, std::string errorMessage)
{
    HRESULT hr = fence.GetFence()->SetEventOnCompletion(m_id, m_fenceEvent);
    if (FAILED(hr)) {
        errorMessage = "Can't attach to Fence!";
        return false;
    }
    return true;
}

void rendering::DXFenceEvent::WaitBlocking()
{
    WaitForSingleObject(m_fenceEvent, INFINITE);
    interpreter::utils::RunCallback(m_callback, interpreter::Value());
}

rendering::DXFenceEvent::~DXFenceEvent()
{
    if (m_waitThread) {
        delete m_waitThread;
    }
    m_waitThread = nullptr;
}

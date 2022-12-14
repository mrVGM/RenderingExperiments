#include "dxFenceEvent.h"

#include "dxFence.h"
#include "nativeFunc.h"
#include "utils.h"
#include "value.h"

#include "threadPool.h"

#include <semaphore>
#include <list>

namespace
{
    struct FenceEventWait : public rendering::threadPool::Runnable
    {
        interpreter::Value m_fenceEvent;
        interpreter::Value m_callback;

        void Run() override
        {
            rendering::DXFenceEvent* fe =
                static_cast<rendering::DXFenceEvent*>(interpreter::NativeObject::ExtractNativeObject(m_fenceEvent));

            fe->WaitBlocking(m_callback);
        }
        void Ready() override;
    };
    std::list<FenceEventWait> m_callEventsCreated;

    void FenceEventWait::Ready()
    {
        for (std::list<FenceEventWait>::iterator it = m_callEventsCreated.begin(); it != m_callEventsCreated.end(); ++it) {
            FenceEventWait& cur = *it;
            if (&cur == this) {
                m_callEventsCreated.erase(it);
                break;
            }
        }
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
    wait = CreateNativeMethod(nativeObject, 3, [&](Value scope) {
        Value self = scope.GetProperty("self");
        DXFenceEvent* fenceEvent = static_cast<DXFenceEvent*>(NativeObject::ExtractNativeObject(self));

        Value fenceValue = scope.GetProperty("param0");
        DXFence* fence = dynamic_cast<DXFence*>(NativeObject::ExtractNativeObject(fenceValue));
        if (!fence) {
            THROW_EXCEPTION("Please supply a Fence!");
        }

        Value triggerValue = scope.GetProperty("param1");
        if (triggerValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply trigger value!");
        }

        Value func = scope.GetProperty("param2");
        if (func.GetType() != ScriptingValueType::Object) {
            THROW_EXCEPTION("Please supply a Callback!");
        }

        fenceEvent->m_id = static_cast<int>(triggerValue.GetNum());

        std::string error;
        bool res = fenceEvent->AttachToFence(*fence, error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        m_callEventsCreated.push_back(FenceEventWait());
        FenceEventWait& ce = m_callEventsCreated.back();
        ce.m_fenceEvent = self;
        ce.m_callback = func;

        threadPool::StartRoutine(&ce);

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

void rendering::DXFenceEvent::WaitBlocking(const interpreter::Value& callback)
{
    WaitForSingleObject(m_fenceEvent, INFINITE);
    interpreter::utils::RunCallback(callback, interpreter::Value());
}

rendering::DXFenceEvent::~DXFenceEvent()
{
    if (m_waitThread) {
        m_waitThread->join();
        delete m_waitThread;
    }
    m_waitThread = nullptr;
}

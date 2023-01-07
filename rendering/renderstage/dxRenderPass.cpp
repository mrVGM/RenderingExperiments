#include "renderstage/dxRenderPass.h"

#include "nativeFunc.h"

#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxSwapChain.h"
#include "dxBuffer.h"
#include "dxDescriptorHeap.h"
#include "dxCommandQueue.h"
#include "dxFence.h"

#include <set>

void rendering::renderstage::DXRenderPass::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& setType = GetOrCreateProperty(nativeObject, "setType");
    setType = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXRenderPass* self = static_cast<DXRenderPass*>(NativeObject::ExtractNativeObject(selfValue));

        Value typeValue = scope.GetProperty("param0");
        if (typeValue.GetType() != ScriptingValueType::String) {
            THROW_EXCEPTION("Please supply Render Pass Type");
        }

        if (typeValue.GetString() == "Canvas") {
            self->m_type = rendering::material::MaterialType::Canvas;
            return Value();
        }

        if (typeValue.GetString() == "Cloud") {
            self->m_type = rendering::material::MaterialType::Cloud;
            return Value();
        }

        if (typeValue.GetString() == "Lit") {
            self->m_type = rendering::material::MaterialType::Lit;
            return Value();
        }

        if (typeValue.GetString() == "Sky") {
            self->m_type = rendering::material::MaterialType::Sky;
            return Value();
        }

        if (typeValue.GetString() == "Unlit") {
            self->m_type = rendering::material::MaterialType::Unlit;
            return Value();
        }

        THROW_EXCEPTION("Please supply a valid Render Pass Type");
    });

#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

bool rendering::renderstage::DXRenderPass::Init(DXRenderer& renderer, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    THROW_ERROR(
        renderer.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        renderer.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandListStart)),
        "Can't create Command List Start!")

    THROW_ERROR(
        m_commandListStart->Close(),
        "Can't close command List Srart!")


    THROW_ERROR(
        renderer.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandListEnd)),
        "Can't create Command List End!")

    THROW_ERROR(
        m_commandListEnd->Close(),
        "Can't close command List End!")

    return true;
}

bool rendering::renderstage::DXRenderPass::Execute(DXRenderer& renderer, std::string& errorMessage)
{
    THROW_ERROR(
        m_commandAllocator->Reset(),
        "Can't reset Command Allocator!")

    THROW_ERROR(
        m_commandListStart->Reset(m_commandAllocator.Get(), nullptr),
        "Can't reset Command List Start!")

    ID3D12Resource* renderTarget = renderer.GetISwapChain()->GetCurrentRenderTarget();
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandListStart->ResourceBarrier(1, &barrier);
    }

    THROW_ERROR(
        m_commandListStart->Close(),
        "Can't close command List Srart!")


    THROW_ERROR(
        m_commandListEnd->Reset(m_commandAllocator.Get(), nullptr),
        "Can't reset Command List End!")

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_commandListEnd->ResourceBarrier(1, &barrier);
    }

    THROW_ERROR(
        m_commandListEnd->Close(),
        "Can't close command List Srart!")


    ID3D12CommandList* ppCommandLists[] = { m_commandListStart.Get(), m_commandListEnd.Get() };
    renderer.GetCommandQueue()->ExecuteCommandLists(1, ppCommandLists);

    bool res = RenderMaterials(renderer, errorMessage);
    if (!res) {
        return false;
    }

    renderer.GetCommandQueue()->ExecuteCommandLists(1, ppCommandLists + 1);

    return true;
}

bool rendering::renderstage::DXRenderPass::RenderMaterials(rendering::DXRenderer& renderer, std::string& errorMessage)
{
    std::set<int> alreadyRendered;

    scene::IScene* scene = renderer.GetScene();
    scene::IMeshRepo* meshRepo = renderer.GetMeshRepo();
    material::IMaterialRepo* materialRepo = renderer.GetMaterialRepo();

    for (std::map<std::string, scene::Object3D>::iterator it = scene->m_objects.begin(); it != scene->m_objects.end(); ++it) {
        scene::Object3D& cur = it->second;

        if (alreadyRendered.contains(cur.m_instanceBufferID)) {
            continue;
        }

        alreadyRendered.insert(cur.m_instanceBufferID);

        scene::InstanceBuffer& instanceBuffer = scene->m_instanceBuffers[cur.m_instanceBufferID];

        std::map<std::string, scene::Mesh>::iterator meshIt = meshRepo->m_meshes.find(cur.m_mesh);
        if (meshIt == meshRepo->m_meshes.end()) {
            errorMessage = "Can't find Mesh!";
            return false;
        }

        scene::Mesh& mesh = meshIt->second;

        std::list<material::IMaterial*> materials;
        for (std::list<std::string>::const_iterator materialNameIt = cur.m_materials.begin(); materialNameIt != cur.m_materials.end(); ++materialNameIt) {
            const std::string& matName = *materialNameIt;

            std::map<std::string, material::IMaterial*>::iterator materialIt = materialRepo->m_materials.find(matName);
            if (materialIt == materialRepo->m_materials.end()) {
                errorMessage = "Can't find material!";
                return false;
            }

            if (materialIt->second->GetMaterialType() == m_type) {
                materials.push_back(materialIt->second);
            }
        }

        for (std::list<material::IMaterial*>::iterator materialIt = materials.begin(); materialIt != materials.end(); ++materialIt) {
            material::IMaterial* curMat = *materialIt;

            bool res = curMat->Render(
                &renderer,
                mesh.m_vertexBuffer,
                mesh.m_vertexBufferSize,
                mesh.m_vertexBufferStride,

                instanceBuffer.m_buffer,
                instanceBuffer.m_size,
                instanceBuffer.m_stride,

                mesh.m_indexBuffer,
                mesh.m_indexBufferSize,
                errorMessage);

            if (!res) {
                return false;
            }
        }
    }

    return true;

}

#undef THROW_ERROR
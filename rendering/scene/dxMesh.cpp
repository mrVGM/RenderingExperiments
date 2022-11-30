#include "dxMesh.h"

#include "d3dx12.h"
#include "nativeFunc.h"
#include "dxBuffer.h"

#include <list>

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::scene::DXMesh::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& vertexBuffer = GetOrCreateProperty(nativeObject, "vertexBuffer");
	Value& indexBuffer = GetOrCreateProperty(nativeObject, "indexBuffer");

	Value& create = GetOrCreateProperty(nativeObject, "create");
	create = CreateNativeMethod(nativeObject, 3, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXMesh* self = static_cast<DXMesh*>(NativeObject::ExtractNativeObject(selfValue));

		Value vertexBufferValue = scope.GetProperty("param0");
		DXBuffer* vb = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(vertexBufferValue));

		if (!vb) {
			THROW_EXCEPTION("Plaese supply Vertex Buffer!")
		}

		Value indexBufferValue = scope.GetProperty("param1");
		DXBuffer* ib = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(indexBufferValue));

		if (!ib) {
			THROW_EXCEPTION("Plaese supply Index Buffer!")
		}

		std::list<Value> ranges;
		Value materials = scope.GetProperty("param2");
		materials.ToList(ranges);
		for (std::list<Value>::iterator it = ranges.begin(); it != ranges.end(); ++it) {
			Value cur = *it;

			std::list<Value> tmp;
			cur.ToList(tmp);
			if (tmp.size() != 2) {
				THROW_EXCEPTION("Plaese supply material indices!")
			}

			if (tmp.front().GetType() != ScriptingValueType::Number ||
				tmp.back().GetType() != ScriptingValueType::Number) {
				THROW_EXCEPTION("Plaese supply material indices!")
			}

			self->m_mesh.m_materialsMap.push_back(std::pair<int, int>(
				static_cast<int>(tmp.front().GetNum()),
				static_cast<int>(tmp.back().GetNum())
			));
		}

		vertexBuffer = vertexBufferValue;
		self->m_mesh.m_vertexBuffer = vb->GetBuffer();
		self->m_mesh.m_vertexBufferSize = vb->GetBufferWidth();
		self->m_mesh.m_vertexBufferStride = vb->GetStride();

		indexBuffer = indexBufferValue;
		self->m_mesh.m_indexBuffer = ib->GetBuffer();
		self->m_mesh.m_indexBufferSize = ib->GetBufferWidth();


		return Value();
	});

#undef THROW_EXCEPTION
}

const rendering::scene::Mesh& rendering::scene::DXMesh::GetMesh() const
{
	return m_mesh;
}



#undef THROW_ERROR
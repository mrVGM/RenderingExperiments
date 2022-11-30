#include "scene/dxMeshRepo.h"

#include "scene/dxMesh.h"
#include "nativeFunc.h"

#include <list>

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::scene::DXMeshRepo::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& meshes = GetOrCreateProperty(nativeObject, "meshes");

	Value& addMesh = GetOrCreateProperty(nativeObject, "addMesh");
	addMesh = CreateNativeMethod(nativeObject, 2, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXMeshRepo* self = static_cast<DXMeshRepo*>(NativeObject::ExtractNativeObject(selfValue));

		Value meshNameValue = scope.GetProperty("param0");
		if (meshNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Plaese supply Mesh Name!")
		}

		Value meshValue = scope.GetProperty("param1");
		DXMesh* mesh = dynamic_cast<DXMesh*>(NativeObject::ExtractNativeObject(meshValue));
		if (!mesh) {
			THROW_EXCEPTION("Plaese supply Mesh!")
		}

		std::list<Value> tmp;
		if (!meshes.IsNone()) {
			meshes.ToList(tmp);
		}
		tmp.push_back(meshValue);
		meshes = Value::FromList(tmp);

		self->m_meshes.insert(std::pair<std::string, Mesh>(meshNameValue.GetString(), mesh->GetMesh()));
		return Value();
	});

#undef THROW_EXCEPTION
}


#undef THROW_ERROR
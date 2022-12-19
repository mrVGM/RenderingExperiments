#include "materials/dxMaterialRepo.h"

#include "materials/IMaterial.h"

#include "nativeFunc.h"

#include <list>

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::material::DXMaterialRepo::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& materials = GetOrCreateProperty(nativeObject, "materials");

	Value& addMaterial = GetOrCreateProperty(nativeObject, "addMaterial");
	addMaterial = CreateNativeMethod(nativeObject, 2, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXMaterialRepo* self = static_cast<DXMaterialRepo*>(NativeObject::ExtractNativeObject(selfValue));

		Value materialNameValue = scope.GetProperty("param0");
		if (materialNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Plaese supply Material Name!")
		}

		Value materialValue = scope.GetProperty("param1");
		IMaterial* material = dynamic_cast<IMaterial*>(NativeObject::ExtractNativeObject(materialValue));
		if (!material) {
			THROW_EXCEPTION("Plaese supply Material!")
		}

		std::list<Value> tmp;
		if (!materials.IsNone()) {
			materials.ToList(tmp);
		}
		tmp.push_back(materialValue);
		materials = Value::FromList(tmp);

		self->m_materials.insert(std::pair<std::string, IMaterial*>(materialNameValue.GetString(), material));
		return Value();
	});

#undef THROW_EXCEPTION
}


#undef THROW_ERROR
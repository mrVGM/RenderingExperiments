#include "scene/dxScene.h"

#include "nativeFunc.h"

#include <list>

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::scene::DXScene::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& addObject = GetOrCreateProperty(nativeObject, "addObject");
	addObject = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		
		Value meshNameValue = scope.GetProperty("param1");
		if (meshNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply mesh name!")
		}

		Object3D obj;
		obj.m_mesh = meshNameValue.GetString();
		self->m_objects.insert(std::pair<std::string, Object3D>(objectNameValue.GetString(), obj));

		return Value();
	});

	Value& addObjectMaterial = GetOrCreateProperty(nativeObject, "addObjectMaterial");
	addObjectMaterial = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		std::map<std::string, Object3D>::iterator it = self->m_objects.find(objectNameValue.GetString());
		if (it == self->m_objects.end()) {
			THROW_EXCEPTION("Can't find object!")
		}

		Object3D& obj = it->second;

		Value materialNameValue = scope.GetProperty("param1");
		if (materialNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply material name!")
		}

		obj.m_materials.push_back(materialNameValue.GetString());
		return Value();
	});

	Value& setObjectPosition = GetOrCreateProperty(nativeObject, "setObjectPosition");
	setObjectPosition = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		std::map<std::string, Object3D>::iterator it = self->m_objects.find(objectNameValue.GetString());
		if (it == self->m_objects.end()) {
			THROW_EXCEPTION("Can't find object!")
		}

		Object3D& obj = it->second;

		Value positionValue = scope.GetProperty("param1");
		std::list<Value> positionValueList;

		positionValue.ToList(positionValueList);
		if (positionValueList.size() != 3) {
			THROW_EXCEPTION("Please supply position vector!")
		}

		std::list<Value>::iterator pit = positionValueList.begin();
		float x = (*pit).GetNum();
		++pit;
		float y = (*pit).GetNum();
		++pit;
		float z = (*pit).GetNum();

		obj.m_transform.m_position = DirectX::XMVectorSet(x,y,z,1);
		return Value();
	});

	Value& setObjectRotation = GetOrCreateProperty(nativeObject, "setObjectRotation");
	setObjectRotation = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		std::map<std::string, Object3D>::iterator it = self->m_objects.find(objectNameValue.GetString());
		if (it == self->m_objects.end()) {
			THROW_EXCEPTION("Can't find object!")
		}

		Object3D& obj = it->second;

		Value rotationValue = scope.GetProperty("param1");
		std::list<Value> rotationValueList;

		rotationValue.ToList(rotationValueList);
		if (rotationValueList.size() != 4) {
			THROW_EXCEPTION("Please supply rotation quaternion!")
		}

		std::list<Value>::iterator rit = rotationValueList.begin();
		float x = (*rit).GetNum();
		++rit;
		float y = (*rit).GetNum();
		++rit;
		float z = (*rit).GetNum();
		++rit;
		float w = (*rit).GetNum();

		obj.m_transform.m_rotation = DirectX::XMVectorSet(x, y, z, w);
		return Value();
	});

	Value& setObjectScale = GetOrCreateProperty(nativeObject, "setObjectScale");
	setObjectScale = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		std::map<std::string, Object3D>::iterator it = self->m_objects.find(objectNameValue.GetString());
		if (it == self->m_objects.end()) {
			THROW_EXCEPTION("Can't find object!")
		}

		Object3D& obj = it->second;

		Value scaleValue = scope.GetProperty("param1");
		std::list<Value> scaleValueList;

		scaleValue.ToList(scaleValueList);
		if (scaleValueList.size() != 3) {
			THROW_EXCEPTION("Please supply scale vector!")
		}

		std::list<Value>::iterator sit = scaleValueList.begin();
		float x = (*sit).GetNum();
		++sit;
		float y = (*sit).GetNum();
		++sit;
		float z = (*sit).GetNum();

		obj.m_transform.m_scale = DirectX::XMVectorSet(x, y, z, 1);
		return Value();
	});

#undef THROW_EXCEPTION
}


#undef THROW_ERROR
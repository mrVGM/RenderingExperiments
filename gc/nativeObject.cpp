#include "nativeObject.h"

interpreter::NativeObject::NativeObject(INativeObject* object) :
	m_object(object)
{
}

void interpreter::NativeObject::SetProperty(std::string name, Value value)
{
}

interpreter::Value interpreter::NativeObject::GetProperty(std::string name) const
{
	std::map<std::string, Value>::const_iterator it = m_properties.find(name);
	if (it == m_properties.end()) {
		return interpreter::Value();
	}
	return it->second;
}

interpreter::INativeObject& interpreter::NativeObject::GetNativeObject()
{
	return *m_object;
}

interpreter::NativeObject::~NativeObject()
{
	delete m_object;
}

interpreter::Value interpreter::NativeObject::Create(INativeObject* nativeObject)
{
	NativeObject* tmp = new NativeObject(nativeObject);
	Value res(*tmp);
	tmp->m_object = nativeObject;
	tmp->m_object->InitProperties(*tmp);
	return res;
}

interpreter::INativeObject* interpreter::NativeObject::ExtractNativeObject(const interpreter::Value& value)
{
	NativeObject* nativeObject = dynamic_cast<NativeObject*>(value.GetManagedValue());
	if (!nativeObject) {
		return nullptr;
	}

	return &nativeObject->GetNativeObject();
}

void interpreter::INativeObject::InitProperties(NativeObject& nativeObject)
{
}

interpreter::Value& interpreter::INativeObject::GetOrCreateProperty(NativeObject& nativeObject, std::string name)
{
	std::map<std::string, interpreter::Value>::iterator it = nativeObject.m_properties.find(name);

	if (it == nativeObject.m_properties.end()) {
		nativeObject.m_properties[name] = interpreter::Value();
		nativeObject.m_properties[name].SetImplicitRef(nativeObject);
	}

	return nativeObject.m_properties[name];
}

interpreter::INativeObject::~INativeObject()
{
}

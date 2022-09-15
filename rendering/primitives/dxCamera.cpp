#include "primitives/cube.h"

#include "nativeFunc.h"


namespace
{
	static std::list<float> m_vertices;
}

void rendering::primitives::Cube::InitProperties(interpreter::NativeObject& nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& getVertices = GetOrCreateProperty(nativeObject, "getVertices");
	getVertices = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		Cube* self = static_cast<Cube*>(NativeObject::ExtractNativeObject(selfValue));

		if (m_vertices.size() == 0) {
			self->GenerateVertices();
		}

		std::list<Value> tmp;

		for (std::list<float>::const_iterator it = m_vertices.begin(); it != m_vertices.end(); ++it) {
			tmp.push_back(Value(*it));
		}

		Value res = Value::FromList(tmp);
		return res;
	});

#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::primitives::Cube::GenerateVertices() const
{

}

#undef THROW_ERROR
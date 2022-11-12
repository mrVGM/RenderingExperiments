#include "primitives/cube.h"

#include "nativeFunc.h"

#include <DirectXMath.h>

namespace
{
	static std::list<float> m_vertices;
	static std::list<int> m_indices;
	static std::list<int> m_indicesInverted;

	void PushVector(const DirectX::XMVECTOR& vector)
	{
		m_vertices.push_back(DirectX::XMVectorGetX(vector));
		m_vertices.push_back(DirectX::XMVectorGetY(vector));
		m_vertices.push_back(DirectX::XMVectorGetZ(vector));
	}

	void PushUV(float u, float v)
	{
		m_vertices.push_back(u);
		m_vertices.push_back(v);
	}
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

	Value& getIndices = GetOrCreateProperty(nativeObject, "getIndices");
	getIndices = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		Cube* self = static_cast<Cube*>(NativeObject::ExtractNativeObject(selfValue));

		Value invertedValue = scope.GetProperty("param0");
		if (invertedValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply inverted bool!")
		}

		bool inverted = static_cast<int>(invertedValue.GetNum());

		if (m_indices.size() == 0) {
			self->GenerateVertices();
		}

		std::list<Value> tmp;
		std::list<int>* l = &m_indices;
		if (inverted) {
			l = &m_indicesInverted;
		}

		for (std::list<int>::const_iterator it = l->begin(); it != l->end(); ++it) {
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

void rendering::primitives::Cube::GenerateVertices(bool inverted) const
{
	typedef DirectX::XMVECTOR vec;

	int baseIndex = 0;
	{
		vec a = DirectX::XMVectorSet(-1, -1, -1, 1);
		vec b = DirectX::XMVectorSet(1, -1, -1, 1);
		vec c = DirectX::XMVectorSet(1, 1, -1, 1);
		vec d = DirectX::XMVectorSet(-1, 1, -1, 1);
		vec n = DirectX::XMVectorSet(0, 0, -1, 1);

		PushVector(a);
		PushVector(n);
		PushUV(0, 0);

		PushVector(b);
		PushVector(n);
		PushUV(1, 0);

		PushVector(c);
		PushVector(n);
		PushUV(1, 1);

		PushVector(d);
		PushVector(n);
		PushUV(0, 1);

		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 3);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 1);
	}

	{
		vec a = DirectX::XMVectorSet(1, -1, -1, 1);
		vec b = DirectX::XMVectorSet(1, -1, 1, 1);
		vec c = DirectX::XMVectorSet(1, 1, 1, 1);
		vec d = DirectX::XMVectorSet(1, 1, -1, 1);
		vec n = DirectX::XMVectorSet(1, 0, 0, 1);

		PushVector(a);
		PushVector(n);
		PushUV(0, 0);

		PushVector(b);
		PushVector(n);
		PushUV(1, 0);

		PushVector(c);
		PushVector(n);
		PushUV(1, 1);

		PushVector(d);
		PushVector(n);
		PushUV(0, 1);

		baseIndex += 4;

		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 3);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 1);
	}


	{
		vec a = DirectX::XMVectorSet(1, -1, 1, 1);
		vec b = DirectX::XMVectorSet(-1, -1, 1, 1);
		vec c = DirectX::XMVectorSet(-1, 1, 1, 1);
		vec d = DirectX::XMVectorSet(1, 1, 1, 1);
		vec n = DirectX::XMVectorSet(0, 0, 1, 1);

		PushVector(a);
		PushVector(n);
		PushUV(0, 0);

		PushVector(b);
		PushVector(n);
		PushUV(1, 0);

		PushVector(c);
		PushVector(n);
		PushUV(1, 1);

		PushVector(d);
		PushVector(n);
		PushUV(0, 1);

		baseIndex += 4;

		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 3);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 1);
	}

	{
		vec a = DirectX::XMVectorSet(-1, -1, 1, 1);
		vec b = DirectX::XMVectorSet(-1, -1, -1, 1);
		vec c = DirectX::XMVectorSet(-1, 1, -1, 1);
		vec d = DirectX::XMVectorSet(-1, 1, 1, 1);
		vec n = DirectX::XMVectorSet(-1, 0, 0, 1);

		PushVector(a);
		PushVector(n);
		PushUV(0, 0);

		PushVector(b);
		PushVector(n);
		PushUV(1, 0);

		PushVector(c);
		PushVector(n);
		PushUV(1, 1);

		PushVector(d);
		PushVector(n);
		PushUV(0, 1);

		baseIndex += 4;

		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 3);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 1);
	}

	{
		vec a = DirectX::XMVectorSet(-1, 1, -1, 1);
		vec b = DirectX::XMVectorSet(1, 1, -1, 1);
		vec c = DirectX::XMVectorSet(1, 1, 1, 1);
		vec d = DirectX::XMVectorSet(-1, 1, 1, 1);
		vec n = DirectX::XMVectorSet(0, 1, 0, 1);

		PushVector(a);
		PushVector(n);
		PushUV(0, 0);

		PushVector(b);
		PushVector(n);
		PushUV(1, 0);

		PushVector(c);
		PushVector(n);
		PushUV(1, 1);

		PushVector(d);
		PushVector(n);
		PushUV(0, 1);

		baseIndex += 4;

		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 3);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 1);
	}

	{
		vec a = DirectX::XMVectorSet(-1, -1, 1, 1);
		vec b = DirectX::XMVectorSet(1, -1, 1, 1);
		vec c = DirectX::XMVectorSet(1, -1, -1, 1);
		vec d = DirectX::XMVectorSet(-1, -1, -1, 1);
		vec n = DirectX::XMVectorSet(0, -1, 0, 1);

		PushVector(a);
		PushVector(n);
		PushUV(0, 0);

		PushVector(b);
		PushVector(n);
		PushUV(1, 0);

		PushVector(c);
		PushVector(n);
		PushUV(1, 1);

		PushVector(d);
		PushVector(n);
		PushUV(0, 1);

		baseIndex += 4;

		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 3);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 0);
		m_indices.push_back(baseIndex + 2);
		m_indices.push_back(baseIndex + 1);
	}

	std::list<int>::iterator it = m_indices.begin();
	while (it != m_indices.end()) {
		int a = *it;
		++it;
		int b = *it;
		++it;
		int c = *it;
		++it;

		m_indicesInverted.push_back(a);
		m_indicesInverted.push_back(c);
		m_indicesInverted.push_back(b);
	}
}

#undef THROW_ERROR
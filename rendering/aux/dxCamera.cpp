#include "aux/dxCamera.h"

#include "nativeFunc.h"
#include "dxBuffer.h"

#include <list>

void rendering::DXCamera::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& setPosition = GetOrCreateProperty(nativeObject, "setPosition");
	setPosition = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		Value vectorValue = scope.GetProperty("param0");
		std::list<Value> tmp;
		vectorValue.ToList(tmp);

		if (tmp.size() != 3) {
			THROW_EXCEPTION("Please supply 3 numbers!")
		}

		std::vector<float> floatList;
		for (std::list<Value>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
			const Value& cur = *it;
			if (cur.GetType() != ScriptingValueType::Number) {
				THROW_EXCEPTION("Numbers only please!")
			}

			floatList.push_back(cur.GetNum());
		}

		self->m_position = { floatList[0], floatList[1], floatList[2], 1 };
		return Value();
	});

	Value& setTarget = GetOrCreateProperty(nativeObject, "setTarget");
	setTarget = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		Value vectorValue = scope.GetProperty("param0");
		std::list<Value> tmp;
		vectorValue.ToList(tmp);

		if (tmp.size() != 3) {
			THROW_EXCEPTION("Please supply 3 numbers!")
		}

		std::vector<float> floatList;
		for (std::list<Value>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
			const Value& cur = *it;
			if (cur.GetType() != ScriptingValueType::Number) {
				THROW_EXCEPTION("Numbers only please!")
			}

			floatList.push_back(cur.GetNum());
		}

		self->m_target = { floatList[0], floatList[1], floatList[2], 1 };
		return Value();
	});

	Value& setAspect = GetOrCreateProperty(nativeObject, "setAspect");
	setAspect = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		Value aspectValue = scope.GetProperty("param0");
		
		if (aspectValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply a number!")
		}

		self->m_aspect = aspectValue.GetNum();
		return Value();
	});

	Value& setNearPlane = GetOrCreateProperty(nativeObject, "setNearPlane");
	setNearPlane = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		Value nearPlaneValue = scope.GetProperty("param0");

		if (nearPlaneValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply a number!")
		}

		self->m_nearPlane = nearPlaneValue.GetNum();
		return Value();
	});

	Value& setFarPlane = GetOrCreateProperty(nativeObject, "setFarPlane");
	setFarPlane = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		Value nearPlaneValue = scope.GetProperty("param0");

		if (nearPlaneValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply a number!")
		}

		self->m_farPlane = nearPlaneValue.GetNum();
		return Value();
	});

	Value& setFOV = GetOrCreateProperty(nativeObject, "setFOV");
	setFOV = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		Value nearPlaneValue = scope.GetProperty("param0");

		if (nearPlaneValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply a number!")
		}

		self->m_fov = nearPlaneValue.GetNum();
		return Value();
	});

	Value& getMVPMatrix = GetOrCreateProperty(nativeObject, "getMVPMatrix");
	getMVPMatrix = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		std::list<Value> matrixCoefs;
		DirectX::XMMATRIX mvp = DirectX::XMMatrixTranspose(self->GetMVPMatrix());

		for (int r = 0; r < 4; ++r) {
			float x = DirectX::XMVectorGetX(mvp.r[r]);
			float y = DirectX::XMVectorGetY(mvp.r[r]);
			float z = DirectX::XMVectorGetZ(mvp.r[r]);
			float w = DirectX::XMVectorGetW(mvp.r[r]);

			matrixCoefs.push_back(x);
			matrixCoefs.push_back(y);
			matrixCoefs.push_back(z);
			matrixCoefs.push_back(w);
		}

		return Value::FromList(matrixCoefs);
	});

	Value& getForward = GetOrCreateProperty(nativeObject, "getForward");
	getForward = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		DirectX::XMVECTOR fwd = self->GetForwardVector();
		std::list<Value> tmp;
		tmp.push_back(Value(DirectX::XMVectorGetX(fwd)));
		tmp.push_back(Value(DirectX::XMVectorGetY(fwd)));
		tmp.push_back(Value(DirectX::XMVectorGetZ(fwd)));

		return Value::FromList(tmp);
	});

	Value& getRight = GetOrCreateProperty(nativeObject, "getRight");
	getRight = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		DirectX::XMVECTOR right = self->GetRightVector();
		std::list<Value> tmp;
		tmp.push_back(Value(DirectX::XMVectorGetX(right)));
		tmp.push_back(Value(DirectX::XMVectorGetY(right)));
		tmp.push_back(Value(DirectX::XMVectorGetZ(right)));

		return Value::FromList(tmp);
	});

	Value& getPos = GetOrCreateProperty(nativeObject, "getPos");
	getPos = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		DirectX::XMVECTOR pos = self->m_position;
		std::list<Value> tmp;
		tmp.push_back(Value(DirectX::XMVectorGetX(pos)));
		tmp.push_back(Value(DirectX::XMVectorGetY(pos)));
		tmp.push_back(Value(DirectX::XMVectorGetZ(pos)));

		return Value::FromList(tmp);
	});

	Value& getTarget = GetOrCreateProperty(nativeObject, "getTarget");
	getTarget = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		DirectX::XMVECTOR target = self->m_position;
		std::list<Value> tmp;
		tmp.push_back(Value(DirectX::XMVectorGetX(target)));
		tmp.push_back(Value(DirectX::XMVectorGetY(target)));
		tmp.push_back(Value(DirectX::XMVectorGetZ(target)));

		return Value::FromList(tmp);
	});

	Value& camBuff = GetOrCreateProperty(nativeObject, "camBuff");
	
	Value& setCamBuff = GetOrCreateProperty(nativeObject, "setCamBuff");
	setCamBuff = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		Value camBuffValue = scope.GetProperty("param0");
		DXBuffer* camBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(camBuffValue));

		if (!camBuffer) {
			THROW_EXCEPTION("Please supply a cam buffer!")
		}

		camBuff = camBuffValue;
		self->m_camBuff = camBuffer->GetBuffer();

		return Value();
	});

#undef THROW_EXCEPTION
}


#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

DirectX::XMMATRIX rendering::DXCamera::GetMVPMatrix() const
{
	DirectX::XMVECTOR fwd = DirectX::XMVectorSubtract(m_target, m_position);
	
	DirectX::XMVECTOR up{ 0, 1, 0, 1 };
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(up, fwd);
	up = DirectX::XMVector3Cross(fwd, right);

	fwd = DirectX::XMVector3Normalize(fwd);
	right = DirectX::XMVector3Normalize(right);
	up = DirectX::XMVector3Normalize(up);

	float fovRad = DirectX::XMConvertToRadians(m_fov);

	float h = tan(fovRad / 2);
	float w = m_aspect * h;

	DirectX::XMMATRIX translate(
		DirectX::XMVECTOR{ 1, 0, 0, -DirectX::XMVectorGetX(m_position) },
		DirectX::XMVECTOR{ 0, 1, 0, -DirectX::XMVectorGetY(m_position) },
		DirectX::XMVECTOR{ 0, 0, 1, -DirectX::XMVectorGetZ(m_position) },
		DirectX::XMVECTOR{ 0, 0, 0, 1 }
	);

	DirectX::XMMATRIX view(
		DirectX::XMVECTOR{ DirectX::XMVectorGetX(right), DirectX::XMVectorGetY(right), DirectX::XMVectorGetZ(right), 0 },
		DirectX::XMVECTOR{ DirectX::XMVectorGetX(up), DirectX::XMVectorGetY(up), DirectX::XMVectorGetZ(up), 0 },
		DirectX::XMVECTOR{ DirectX::XMVectorGetX(fwd), DirectX::XMVectorGetY(fwd), DirectX::XMVectorGetZ(fwd), 0 },
		DirectX::XMVECTOR{ 0, 0, 0, 1 }
	);


	DirectX::XMMATRIX project(
		DirectX::XMVECTOR{ 1 / w, 0, 0, 0 },
		DirectX::XMVECTOR{ 0, 1 / h, 0, 0 },
		DirectX::XMVECTOR{ 0, 0, m_farPlane / (m_farPlane - m_nearPlane), -m_farPlane * m_nearPlane / (m_farPlane - m_nearPlane)},
		DirectX::XMVECTOR{ 0, 0, 1, 0 }
	);


	return project * view * translate;
}

DirectX::XMVECTOR rendering::DXCamera::GetForwardVector() const
{
	DirectX::XMVECTOR res =  DirectX::XMVectorSubtract(m_target, m_position);
	res = DirectX::XMVector3Normalize(res);
	return res;
}

void rendering::DXCamera::HandleInput(double dt, std::list<WPARAM>& keysDown, std::list<WPARAM>& keysUp)
{
}

DirectX::XMVECTOR rendering::DXCamera::GetRightVector() const
{
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
	DirectX::XMVECTOR fwd = GetForwardVector();
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(up, fwd);

	return DirectX::XMVector3Normalize(right);
}


#undef THROW_ERROR
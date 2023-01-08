#include "helper/dxCamera.h"

#include "nativeFunc.h"
#include "dxBuffer.h"
#include "d3dx12.h"
#include "dxRenderer.h"
#include "api.h"
#include "scene/IScene.h"
#include "helper/dxUpdater.h"
#include "window.h"

#include <list>
#include <corecrt_math_defines.h>

#include <iostream>

namespace
{
	rendering::DXRenderer* GetRenderer()
	{
		using namespace interpreter;

		Value api = rendering::GetAPI();
		Value app_context = api.GetProperty("app_context");

		Value renderer = app_context.GetProperty("renderer");

		if (renderer.IsNone()) {
			return nullptr;
		}

		return static_cast<rendering::DXRenderer*>(NativeObject::ExtractNativeObject(renderer));
	}

	bool UpdateSceneTransform(rendering::DXRenderer* renderer, std::string& errorMessage)
	{
		using namespace rendering;

		if (!renderer) {
			return true;
		}

		scene::IScene* scene = renderer->GetScene();

		for (std::map<int, scene::InstanceBuffer>::iterator it = scene->m_instanceBuffers.begin(); it != scene->m_instanceBuffers.end(); ++it) {
			int id = it->first;
			scene::InstanceBuffer& cur = it->second;


			CD3DX12_RANGE readRange(0, 0);
			void* dst = nullptr;
			if (FAILED(cur.m_buffer->Map(0, &readRange, &dst))) {
				errorMessage = "Can't Map Instance Buffer!";
				return false;
			}
			for (std::map<std::string, scene::Object3D>::const_iterator objIt = scene->m_objects.begin(); objIt != scene->m_objects.end(); ++objIt) {
				const scene::Object3D& curObj = objIt->second;

				if (id != curObj.m_instanceBufferID) {
					continue;
				}

				scene::Transform* tmp = reinterpret_cast<scene::Transform*>(dst);
				memcpy(tmp + curObj.m_instanceBufferOffset, &curObj.m_transform, sizeof(scene::Transform));
			}
			cur.m_buffer->Unmap(0, nullptr);
		}

		if (!scene->UpdateColladaSceneInstanceBuffers(errorMessage)) {
			return false;
		}

		return true;
	}
}

void rendering::DXCamera::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& p_updaters = GetOrCreateProperty(nativeObject, "updaters");

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

	Value& addUpdater = GetOrCreateProperty(nativeObject, "addUpdater");
	addUpdater = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXCamera* self = static_cast<DXCamera*>(NativeObject::ExtractNativeObject(selfValue));

		Value updaterValue = scope.GetProperty("param0");
		helper::DXUpdater* updater = dynamic_cast<helper::DXUpdater*>(NativeObject::ExtractNativeObject(updaterValue));

		if (!updater) {
			THROW_EXCEPTION("Please supply updater!")
		}

		std::list<Value> tmp;
		if (!p_updaters.IsNone()) {
			p_updaters.ToList(tmp);
		}

		tmp.push_back(updaterValue);
		p_updaters = Value::FromList(tmp);

		self->m_updaters.push_back(updater);
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
	DirectX::XMVECTOR right, fwd, up;
	GetCoordinateVectors(right, fwd, up);
	
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

void rendering::DXCamera::GetCoordinateVectors(DirectX::XMVECTOR& right, DirectX::XMVECTOR& fwd, DirectX::XMVECTOR& up) const
{
	fwd = DirectX::XMVectorSubtract(m_target, m_position);

	up = DirectX::XMVectorSet(0, 1, 0, 1);
	right = DirectX::XMVector3Cross(up, fwd);
	up = DirectX::XMVector3Cross(fwd, right);

	fwd = DirectX::XMVector3Normalize(fwd);
	right = DirectX::XMVector3Normalize(right);
	up = DirectX::XMVector3Normalize(up);
}

void rendering::DXCamera::MoveCamera(double dt, const long cursorPos[2])
{
	using namespace DirectX;

	rendering::DXRenderer* renderer = GetRenderer();
	if (!renderer) {
		return;
	}	

	if (m_aiming) {
		m_azimuth = -m_angleSpeed * (cursorPos[0] - m_cursorRelativePos[0]) + m_anglesCache[0];
		while (m_azimuth >= 360) {
			m_azimuth -= 360;
		}
		while (m_azimuth < 0) {
			m_azimuth += 360;
		}

		m_altitude = -m_angleSpeed * (cursorPos[1] - m_cursorRelativePos[1]) + m_anglesCache[1];
		if (m_altitude > 80) {
			m_altitude = 80;
		}

		if (m_altitude < -80) {
			m_altitude = -80;
		}

		float azimuth = M_PI * m_azimuth / 180.0;
		float altitude = M_PI * m_altitude / 180.0;

		XMVECTOR fwdVector = XMVectorSet(cos(azimuth) * cos(altitude), sin(altitude), sin(azimuth) * cos(altitude), 0);
		XMVECTOR rightVector = XMVector3Cross(XMVectorSet(0, 1, 0, 0), fwdVector);
		rightVector = XMVector3Normalize(rightVector);

		XMVECTOR moveVector = XMVectorSet(m_move[0], 0, m_move[1], 0);
		moveVector = XMVector3Normalize(moveVector);
		moveVector = m_moveSpeed * moveVector;
		moveVector = XMVectorAdd(XMVectorGetX(moveVector) * rightVector, XMVectorGetZ(moveVector) * fwdVector);

		if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(moveVector)) < 0.0000001) {
			moveVector += DirectX::XMVectorSet(0, m_moveSpeed * m_move[2], 0, 0);
		}

		m_position = DirectX::XMVectorAdd(m_position, moveVector);
		m_target = DirectX::XMVectorAdd(m_position, fwdVector);
	}

	float matrixCoefs[38];
	DirectX::XMMATRIX mvp = DirectX::XMMatrixTranspose(GetMVPMatrix());

	int index = 0;
	for (int r = 0; r < 4; ++r) {
		float x = DirectX::XMVectorGetX(mvp.r[r]);
		float y = DirectX::XMVectorGetY(mvp.r[r]);
		float z = DirectX::XMVectorGetZ(mvp.r[r]);
		float w = DirectX::XMVectorGetW(mvp.r[r]);

		matrixCoefs[index++] = x;
		matrixCoefs[index++] = y;
		matrixCoefs[index++] = z;
		matrixCoefs[index++] = w;
	}

	matrixCoefs[index++] = DirectX::XMVectorGetX(m_position);
	matrixCoefs[index++] = DirectX::XMVectorGetY(m_position);
	matrixCoefs[index++] = DirectX::XMVectorGetZ(m_position);
	matrixCoefs[index++] = 1;

	DirectX::XMVECTOR right, fwd, up;
	GetCoordinateVectors(right, fwd, up);

	matrixCoefs[index++] = DirectX::XMVectorGetX(right);
	matrixCoefs[index++] = DirectX::XMVectorGetY(right);
	matrixCoefs[index++] = DirectX::XMVectorGetZ(right);
	matrixCoefs[index++] = 1;

	matrixCoefs[index++] = DirectX::XMVectorGetX(fwd);
	matrixCoefs[index++] = DirectX::XMVectorGetY(fwd);
	matrixCoefs[index++] = DirectX::XMVectorGetZ(fwd);
	matrixCoefs[index++] = 1;

	matrixCoefs[index++] = DirectX::XMVectorGetX(up);
	matrixCoefs[index++] = DirectX::XMVectorGetY(up);
	matrixCoefs[index++] = DirectX::XMVectorGetZ(up);
	matrixCoefs[index++] = 1;

	m_time += dt;
	matrixCoefs[index++] = m_fov;
	matrixCoefs[index++] = m_aspect;
	matrixCoefs[index++] = static_cast<float>(m_time);
	matrixCoefs[index++] = m_airAbsorbtion;

	matrixCoefs[index++] = m_sunAzimuth;
	matrixCoefs[index++] = m_sunAltitude;

	CD3DX12_RANGE readRange(0, 0);
	void* dst = nullptr;
	if (FAILED(renderer->GetCamBuff()->Map(0, &readRange, &dst))) {
		return;
	}
	memcpy(dst, matrixCoefs, _countof(matrixCoefs) * sizeof(float));
	renderer->GetCamBuff()->Unmap(0, nullptr);
}

void rendering::DXCamera::HandleInput(double dt, const InputInfo& inputInfo)
{
	rendering::DXRenderer* renderer = GetRenderer();
	if (!renderer) {
		return;
	}

	m_move[0] = 0;
	m_move[1] = 0;
	m_move[2] = 0;
	for (std::set<WPARAM>::const_iterator it = inputInfo.m_keysDown.begin(); it != inputInfo.m_keysDown.end(); ++it) {
		WPARAM x = *it;
		if (x == 65) {
			m_move[0] = -1;
		}
		if (x == 68) {
			m_move[0] = 1;
		}

		if (x == 87) {
			m_move[1] = 1;
		}
		if (x == 83) {
			m_move[1] = -1;
		}

		if (x == 81) {
			m_move[2] = -1;
		}
		if (x == 69) {
			m_move[2] = 1;
		}
	}

	RECT rect;
	GetWindowRect(inputInfo.m_source->m_hwnd, &rect);

	if (inputInfo.m_rightMouseButtonDown && !m_aiming) {
		m_cursorRelativePos[0] = inputInfo.m_mouseMovement[0];
		m_cursorRelativePos[1] = inputInfo.m_mouseMovement[1];
		m_anglesCache[0] = m_azimuth;
		m_anglesCache[1] = m_altitude;

		ClipCursor(&rect);
		ShowCursor(false);
	}

	if (!inputInfo.m_rightMouseButtonDown && m_aiming) {
		ClipCursor(nullptr);
		ShowCursor(true);
	}

	m_aiming = inputInfo.m_rightMouseButtonDown;
	if (m_aiming) {
		SetCursorPos((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
	}

	MoveCamera(dt, inputInfo.m_mouseMovement);
	RunUpdaters(dt);

	std::string error;
	UpdateSceneTransform(renderer, error);
}

void rendering::DXCamera::RunUpdaters(double dt)
{
	for (std::list<helper::IUpdater*>::iterator it = m_updaters.begin(); it != m_updaters.end(); ++it) {
		helper::IUpdater* cur = *it;

		cur->Update(dt);
	}
}


#undef THROW_ERROR

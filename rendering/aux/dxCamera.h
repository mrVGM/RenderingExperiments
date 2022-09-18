#pragma once

#include "nativeObject.h"

#include <DirectXMath.h>

namespace rendering
{
	class DXCamera : public interpreter::INativeObject
	{
		float m_fov = 60;
		float m_aspect = 1;

		float m_nearPlane = 0.1;
		float m_farPlane = 1000;


		DirectX::XMVECTOR m_position;
		DirectX::XMVECTOR m_target;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		DirectX::XMMATRIX GetMVPMatrix() const;
	public:
	};
}
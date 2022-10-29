#pragma once

#include "nativeObject.h"

#include "inputHandler.h"

#include <DirectXMath.h>

namespace rendering
{
	class DXCamera : public interpreter::INativeObject, public InputHandler
	{
		float m_fov = 60;
		float m_aspect = 1;

		float m_nearPlane = 0.1;
		float m_farPlane = 1000;


		DirectX::XMVECTOR m_position;
		DirectX::XMVECTOR m_target;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		DirectX::XMMATRIX GetMVPMatrix() const;
		
		DirectX::XMVECTOR GetForwardVector() const;
		DirectX::XMVECTOR GetRightVector() const;
	public:
		void HandleInput(double dt, std::list<WPARAM>& keysDown, std::list<WPARAM>& keysUp) override;
	};
}
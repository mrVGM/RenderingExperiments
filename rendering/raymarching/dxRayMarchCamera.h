#pragma once

#include "nativeObject.h"

#include "helper/inputHandler.h"

#include <d3d12.h>
#include <DirectXMath.h>

namespace rendering::raymarch
{
	class DXRayMarchCamera : public interpreter::INativeObject, public InputHandler
	{
		ID3D12Resource* m_camBuff = nullptr;

		float m_angleSpeed = 40;
		float m_moveSpeed = 0.1;

		float m_azimuth = 90;
		float m_altitude = 0;

		float m_fov = 60;
		float m_aspect = 1;

		float m_nearPlane = 0.1;
		float m_farPlane = 1000;

		float m_sunAngle = 0;

		float m_cloudDisplacement = 0;

		DirectX::XMVECTOR m_position;
		DirectX::XMVECTOR m_target;

		float m_tick = 0;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		DirectX::XMMATRIX GetMVPMatrix() const;
		
		DirectX::XMVECTOR GetForwardVector() const;
		DirectX::XMVECTOR GetRightVector() const;
	public:
		void HandleInput(double dt, std::list<WPARAM>& keysDown, std::list<WPARAM>& keysUp) override;
		void RunUpdaters(double dt) override;
	};
}

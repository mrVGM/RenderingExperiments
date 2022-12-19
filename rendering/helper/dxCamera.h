#pragma once

#include "nativeObject.h"

#include "inputHandler.h"
#include "iUpdater.h"

#include <d3d12.h>
#include <DirectXMath.h>

namespace rendering
{
	class DXCamera : public interpreter::INativeObject, public InputHandler
	{
		float m_airAbsorbtion = 0.01;

		float m_angleSpeed = 300;
		float m_moveSpeed = 1;

		float m_azimuth = 90;
		float m_altitude = 0;

		float m_fov = 60;
		float m_aspect = 1;

		float m_nearPlane = 0.1;
		float m_farPlane = 1000;

		DirectX::XMVECTOR m_position;
		DirectX::XMVECTOR m_target;

		double m_time = 0;

		std::list<helper::IUpdater*> m_updaters;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		DirectX::XMMATRIX GetMVPMatrix() const;
		
		DirectX::XMVECTOR GetForwardVector() const;
		DirectX::XMVECTOR GetRightVector() const;
	public:
		void HandleInput(double dt, const InputInfo& inputInfo) override;
		void RunUpdaters(double dt) override;
	};
}

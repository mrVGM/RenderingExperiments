#pragma once

#include "nativeObject.h"

#include "helper/iUpdater.h"

#include <d3d12.h>
#include <DirectXMath.h>

namespace rendering::helper
{
	class DXUpdater : public interpreter::INativeObject, public IUpdater
	{
		double m_totalTime = 0;
		double m_timeSinceUpdate = 1000;
		double m_updateTime = 1;

		ID3D12Resource* m_settingsBuffer;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

	public:
		void Update(double dt) override;
	};
}

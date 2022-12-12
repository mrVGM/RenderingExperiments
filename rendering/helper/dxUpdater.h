#pragma once

#include "nativeObject.h"

#include "helper/iUpdater.h"

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <list>

namespace rendering::helper
{
	class DXUpdater : public interpreter::INativeObject, public IUpdater
	{
		struct Setting
		{
			std::string m_name;
			int m_dim = 1;
			float m_value[4];
		};

		std::list<Setting> m_settings;

		double m_totalTime = 0;
		double m_timeSinceUpdate = 1000;
		double m_updateTime = 1;

		ID3D12Resource* m_settingsBuffer;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		void UpdateSettings();

	public:
		void Update(double dt) override;
		DXUpdater();
	};
}

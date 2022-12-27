#pragma once

#include "nativeObject.h"

#include "helper/iUpdater.h"
#include "helper/inputHandler.h"

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <list>

#include <thread>

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

		std::map<std::string, Setting> m_settings;
		std::list<std::string> m_shaderSettings;

		double m_totalTime = 0;
		double m_updateTime = 1;

		ID3D12Resource* m_settingsBuffer = nullptr;
		InputHandler* m_camera = nullptr;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		void UpdateSettings();

		void AddSetting(const Setting& setting);

	public:
		void UpdateSettings(const std::string& setting);
		std::thread* m_pipeThread = nullptr;

		void Update(double dt) override;
		DXUpdater();
		~DXUpdater();
	};
}

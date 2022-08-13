#pragma once

#include "value.h"

namespace interpreter
{
	class NativeObject;
	class INativeObject
	{
		friend class NativeObject;
		virtual void InitProperties(NativeObject& nativeObject);
	protected:
		Value& GetOrCreateProperty(NativeObject& nativeObject, std::string name);
	public:
		virtual ~INativeObject();
	};

	class NativeObject : public IManagedValue
	{
		friend class INativeObject;
		std::map<std::string, Value>m_properties;
		INativeObject* m_object;
	protected:
		NativeObject(INativeObject* object);

	public:
		static Value Create(INativeObject* nativeObject);

		void SetProperty(std::string name, Value value) override;
		Value GetProperty(std::string name) const override;

		INativeObject& GetNativeObject();
		~NativeObject();
	};
}
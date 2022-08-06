#pragma once

#include <string>
#include <vector>
#include <map>

namespace interpreter
{
	enum ScriptingValueType
	{
		None,
		Number,
		String,
		Object,
	};

	class GarbageCollector;

	struct IManagedValue;

	class ValueWrapper
	{
		IManagedValue* m_value = nullptr;
		double m_number = 0.0;
		std::string m_string;

		ScriptingValueType m_type = ScriptingValueType::None;


		void Copy(const ValueWrapper& other);

		bool m_explicitRef = true;
		IManagedValue* m_implicitRef = nullptr;
	public:
		ValueWrapper();
		ValueWrapper(double num);
		ValueWrapper(std::string str);

		ValueWrapper(IManagedValue& value);
		ValueWrapper(IManagedValue& value, ScriptingValueType managedValueType);
		ValueWrapper(const ValueWrapper& other);
		ValueWrapper& operator=(const ValueWrapper& other);
		~ValueWrapper();

		bool Equals(const ValueWrapper& other) const;
		std::string ToString() const;

		bool IsManaged() const;
		bool IsNone() const;

		void SetImplicitRef(IManagedValue* implicitRef);

		double GetNum() const;
		std::string GetString() const;
		IManagedValue* GetManagedValue() const;

		ScriptingValueType GetType() const;
		void SetProperty(std::string name, ValueWrapper value);
		ValueWrapper GetProperty(std::string name);
	};

	struct IManagedValue
	{
		virtual void SetProperty(std::string name, ValueWrapper value) = 0;
		virtual ValueWrapper GetProperty(std::string name) const = 0;
	};
}
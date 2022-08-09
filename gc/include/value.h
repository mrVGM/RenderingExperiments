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

	class Value
	{
		IManagedValue* m_value = nullptr;
		double m_number = 0.0;
		std::string m_string;

		ScriptingValueType m_type = ScriptingValueType::None;


		void Copy(const Value& other);

		bool m_explicitRef = true;
		IManagedValue* m_implicitRef = nullptr;
	public:
		Value();
		Value(double num);
		Value(std::string str);

		Value(IManagedValue& value);
		Value(IManagedValue& value, ScriptingValueType managedValueType);
		Value(const Value& other);
		Value& operator=(const Value& other);
		~Value();

		std::string ToString() const;

		bool IsManaged() const;
		bool IsNone() const;

		void SetImplicitRef(IManagedValue* implicitRef);

		double GetNum() const;
		std::string GetString() const;
		IManagedValue* GetManagedValue() const;

		ScriptingValueType GetType() const;
		void SetProperty(std::string name, Value value);
		Value GetProperty(std::string name);

		bool Equals(const Value& other) const;
		bool IsTrue() const;

		static Value Plus(const Value& v1, const Value& v2);
		static Value Minus(const Value& v1, const Value& v2);
		static Value Multiply(const Value& v1, const Value& v2);
		static Value Divide(const Value& v1, const Value& v2);
		static Value Quotient(const Value& v1, const Value& v2);
		static Value Negate(const Value& v1);
		static Value Equal(const Value& v1, const Value& v2);
		static Value Less(const Value& v1, const Value& v2);
		static Value LessOrEqual(const Value& v1, const Value& v2);
		static Value Greater(const Value& v1, const Value& v2);
		static Value GreaterOrEqual(const Value& v1, const Value& v2);
		static Value And(const Value& v1, const Value& v2);
		static Value Or(const Value& v1, const Value& v2);
		static Value Not(const Value& v1);
	};

	struct IManagedValue
	{
		virtual void SetProperty(std::string name, Value value) = 0;
		virtual Value GetProperty(std::string name) const = 0;
		virtual ~IManagedValue();
	};
}
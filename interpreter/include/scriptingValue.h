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

		bool Equals(const ValueWrapper& other) const;
		bool IsTrue() const;

		static ValueWrapper Plus(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Minus(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Multiply(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Divide(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Quotient(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Negate(const ValueWrapper& v1);
		static ValueWrapper Equal(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Less(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper LessOrEqual(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Greater(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper GreaterOrEqual(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper And(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Or(const ValueWrapper& v1, const ValueWrapper& v2);
		static ValueWrapper Not(const ValueWrapper& v1);
	};

	struct IManagedValue
	{
		virtual void SetProperty(std::string name, ValueWrapper value) = 0;
		virtual ValueWrapper GetProperty(std::string name) const = 0;
	};
}
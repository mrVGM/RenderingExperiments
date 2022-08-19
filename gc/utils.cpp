#include "utils.h"
#include "object.h"
#include "ISession.h"

interpreter::Value interpreter::utils::GetEmptyObject()
{
	return interpreter::ObjectValue::Create();
}

void interpreter::utils::RunCallback(const Value& func, const Value& args)
{
	ISession* session = GetSession();

	if (!session) {
		return;
	}

	session->RunCallback(func, args);
}
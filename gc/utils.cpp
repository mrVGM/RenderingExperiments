#include "utils.h"
#include "object.h"
#include "ISession.h"

interpreter::Value interpreter::utils::GetEmptyObject()
{
	return interpreter::ObjectValue::Create();
}

void interpreter::utils::RunFunc(const interpreter::Value& func)
{
	ISession* session = GetSession();

	if (!session) {
		return;
	}

	session->RunFunc(func);
}
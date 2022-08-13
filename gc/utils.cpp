#include "utils.h"
#include "object.h"

interpreter::Value interpreter::utils::GetEmptyObject()
{
	return interpreter::ObjectValue::Create();
}

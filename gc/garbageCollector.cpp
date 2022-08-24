#include "garbageCollector.h"

#include <queue>

interpreter::GarbageCollector* interpreter::GarbageCollector::m_instance = nullptr;

interpreter::GarbageCollector::GarbageCollector()
{
}

interpreter::GarbageCollector& interpreter::GarbageCollector::GetInstance()
{
	if (!m_instance) {
		m_instance = new GarbageCollector();
	}

	return *m_instance;
}

void interpreter::GarbageCollector::DisposeGC()
{
	if (m_instance) {
		delete m_instance;
	}

	m_instance = nullptr;
}

interpreter::GarbageCollector::ManagedValue::~ManagedValue()
{
	delete m_managedValue;
}

interpreter::GarbageCollector::ManagedValue& interpreter::GarbageCollector::FindOrCreateValue(IManagedValue* managedValue)
{
	GarbageCollector::ManagedValue* val = FindValue(managedValue);
	if (val) {
		return *val;
	}

	ManagedValue* res = new ManagedValue();
	res->m_managedValue = managedValue;
	m_allValues.push_back(res);

	return *res;
}

interpreter::GarbageCollector::ManagedValue* interpreter::GarbageCollector::FindValue(IManagedValue* managedValue)
{
	for (int i = 0; i < m_allValues.size(); ++i) {
		if (m_allValues[i]->m_managedValue == managedValue) {
			return m_allValues[i];
		}
	}

	return nullptr;
}

void interpreter::GarbageCollector::CollectGarbage()
{
	if (m_collectingGarbage) {
		return;
	}

	if (m_instructionsBatches > 0) {
		return;
	}

	m_collectingGarbage = true;

	std::queue<ManagedValue*> q;

	for (int i = 0; i < m_allValues.size(); ++i) {
		ManagedValue* cur = m_allValues[i];

		cur->m_visited = false;

		if (cur->m_explicitRefs > 0) {
			q.push(cur);
		}
	}

	while (!q.empty()) {
		ManagedValue* cur = q.front();
		q.pop();

		if (cur->m_visited) {
			continue;
		}

		cur->m_visited = true;
		for (int i = 0; i < cur->m_implicitRefs.size(); ++i) {
			ManagedValue* implicitRef = FindValue(cur->m_implicitRefs[i]);
			q.push(implicitRef);
		}
	}

	std::vector<ManagedValue*> alive;
	std::vector<ManagedValue*> dead;
	for (int i = 0; i < m_allValues.size(); ++i) {
		if (m_allValues[i]->m_visited) {
			alive.push_back(m_allValues[i]);
		}
		else {
			dead.push_back(m_allValues[i]);
		}
	}

	m_allValues = alive;

	for (int i = 0; i < dead.size(); ++i) {
		delete dead[i];
	}

	m_collectingGarbage = false;
}

void interpreter::GarbageCollector::AddExplicitRef(IManagedValue* value)
{
	//m_mutex.lock();

	ManagedValue& mv = FindOrCreateValue(value);
	++mv.m_explicitRefs;

	CollectGarbage();

	//m_mutex.unlock();
}

void interpreter::GarbageCollector::RemoveExplicitRef(IManagedValue* value)
{
	//m_mutex.lock();

	ManagedValue* mv = FindValue(value);
	--mv->m_explicitRefs;

	CollectGarbage();

	//m_mutex.unlock();
}

interpreter::GarbageCollector::~GarbageCollector()
{
	for (int i = 0; i < m_allValues.size(); ++i) {
		delete m_allValues[i];
	}
}

void interpreter::GarbageCollector::AddImplicitRef(IManagedValue* value, IManagedValue* referencedBy)
{
	//m_mutex.lock();
	
	ManagedValue* refBy = FindValue(referencedBy);
	refBy->m_implicitRefs.push_back(value);
	CollectGarbage();

	//m_mutex.unlock();
}

void interpreter::GarbageCollector::RemoveImplicitRef(IManagedValue* value, IManagedValue* referencedBy)
{
	//m_mutex.lock();

	ManagedValue* refBy = FindValue(referencedBy);
	if (!refBy) {
		return;
	}

	for (std::vector<IManagedValue*>::iterator it = refBy->m_implicitRefs.begin(); it != refBy->m_implicitRefs.end(); ++it) {
		if ((*it) == value) {
			refBy->m_implicitRefs.erase(it);
			break;
		}
	}

	CollectGarbage();

	//m_mutex.unlock();
}

interpreter::GarbageCollector::GCInstructionsBatch::GCInstructionsBatch()
{
	++GetInstance().m_instructionsBatches;
}

interpreter::GarbageCollector::GCInstructionsBatch::~GCInstructionsBatch()
{
	--GetInstance().m_instructionsBatches;
	GetInstance().CollectGarbage();
}

interpreter::IManagedValue::~IManagedValue()
{
}

interpreter::IManagedValue::IManagedValue()
{
}

#include "garbageCollector.h"

#include <queue>
#include <thread>

namespace
{
	bool m_running = false;
	std::thread* m_thread = nullptr;

	void run()
	{
		while (m_running) {
			interpreter::GarbageCollector::GetInstance().CollectGarbage();
		}
	}
}

interpreter::GarbageCollector* interpreter::GarbageCollector::m_instance = nullptr;

interpreter::GarbageCollector::GarbageCollector()
{
	m_submitted = &m_commands1;
	m_running = true;
	m_thread = new std::thread(run);
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
	volatile GarbageCollector::GCLock::GCControlLockObject controlLockObject;

	std::vector<GCCommand>* m_currentCommands = nullptr;

	GetInstance().m_gcLock.GCBatchLock();
	GetInstance().m_gcLock.GCInstructionsLock();
	
	m_currentCommands = m_submitted;
	if (m_submitted == &m_commands1) {
		m_submitted = &m_commands2;
	}
	else {
		m_submitted = &m_commands1;
	}
	m_submitted->clear();
	
	GetInstance().m_gcLock.GCInstructionsRelease();
	GetInstance().m_gcLock.GCBatchRelease();
	
	if (m_currentCommands->empty()) {
		return;
	}

	for (int i = 0; i < m_currentCommands->size(); ++i) {
		GCCommand& cur = (*m_currentCommands)[i];
		switch(cur.m_type) {
		case GCCommandType::GCAddExplicitRef:
		{
			ManagedValue& mv = FindOrCreateValue(cur.value1);
			++mv.m_explicitRefs;
			break;
		}
		case GCCommandType::GCRemoveExplicitRef:
		{
			ManagedValue* mv = FindValue(cur.value1);
			--mv->m_explicitRefs;
			break;
		}
		case GCCommandType::GCAddImplicitRef:
		{
			ManagedValue* refBy = FindValue(cur.value2);
			refBy->m_implicitRefs.push_back(cur.value1);
			break;
		}
		case GCCommandType::GCRemoveImplicitRef:
		{
			ManagedValue* refBy = FindValue(cur.value2);
			if (!refBy) {
				break;
			}

			for (std::vector<IManagedValue*>::iterator it = refBy->m_implicitRefs.begin(); it != refBy->m_implicitRefs.end(); ++it) {
				if ((*it) == cur.value1) {
					refBy->m_implicitRefs.erase(it);
					break;
				}
			}
			break;
		}
		}
	}

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
}

void interpreter::GarbageCollector::AddExplicitRef(IManagedValue* value)
{
	GetInstance().m_gcLock.AppInstructionsLock();
	
	GCCommand gcCommand;
	gcCommand.m_type = GCCommandType::GCAddExplicitRef;
	gcCommand.value1 = value;
	m_submitted->push_back(gcCommand);
	
	GetInstance().m_gcLock.AppInstructionsRelease();
}

void interpreter::GarbageCollector::RemoveExplicitRef(IManagedValue* value)
{
	GetInstance().m_gcLock.AppInstructionsLock();

	GCCommand gcCommand;
	gcCommand.m_type = GCCommandType::GCRemoveExplicitRef;
	gcCommand.value1 = value;
	m_submitted->push_back(gcCommand);

	GetInstance().m_gcLock.AppInstructionsRelease();
}

interpreter::GarbageCollector::~GarbageCollector()
{
	m_running = false;
	m_thread->join();
	delete m_thread;
	m_thread = nullptr;

	for (int i = 0; i < m_allValues.size(); ++i) {
		delete m_allValues[i];
	}
}

void interpreter::GarbageCollector::AddImplicitRef(IManagedValue* value, IManagedValue* referencedBy)
{
	GetInstance().m_gcLock.AppInstructionsLock();
	
	GCCommand gcCommand;
	gcCommand.m_type = GCCommandType::GCAddImplicitRef;
	gcCommand.value1 = value;
	gcCommand.value2 = referencedBy;
	m_submitted->push_back(gcCommand);

	GetInstance().m_gcLock.AppInstructionsRelease();
}

void interpreter::GarbageCollector::RemoveImplicitRef(IManagedValue* value, IManagedValue* referencedBy)
{
	GetInstance().m_gcLock.AppInstructionsLock();

	GCCommand gcCommand;
	gcCommand.m_type = GCCommandType::GCRemoveImplicitRef;
	gcCommand.value1 = value;
	gcCommand.value2 = referencedBy;
	m_submitted->push_back(gcCommand);

	GetInstance().m_gcLock.AppInstructionsRelease();
}

interpreter::GarbageCollector::GCInstructionsBatch::GCInstructionsBatch()
{
	GetInstance().m_gcLock.AppBatchLock();
}

interpreter::GarbageCollector::GCInstructionsBatch::~GCInstructionsBatch()
{
	GetInstance().m_gcLock.AppBatchRelease();
}

interpreter::IManagedValue::~IManagedValue()
{
}

interpreter::IManagedValue::IManagedValue()
{
}

void interpreter::GarbageCollector::GCLock::AppInstructionsLock()
{
	if (!m_batchOperation) {
		AppControlLock();
	}

	m_changeInstructionsMutex.lock();
}

void interpreter::GarbageCollector::GCLock::AppInstructionsRelease()
{
	if (!m_batchOperation) {
		AppControlRelease();
	}

	m_changeInstructionsMutex.unlock();
}

void interpreter::GarbageCollector::GCLock::AppBatchLock()
{
	m_batchOperation = true;

	AppControlLock();
	m_batchMutex.lock();
}

void interpreter::GarbageCollector::GCLock::AppBatchRelease()
{
	m_batchMutex.unlock();
	AppControlRelease();

	m_batchOperation = false;
}

void interpreter::GarbageCollector::GCLock::AppControlLock()
{
	GarbageCollector& gc = GarbageCollector::GetInstance();

	if (gc.m_submitted->size() < 500) {
		return;
	}

	gc.m_gcLock.m_controlMutex.lock();
	gc.m_gcLock.m_controlLocked = true;
}

void interpreter::GarbageCollector::GCLock::AppControlRelease()
{
	GarbageCollector& gc = GarbageCollector::GetInstance();
	if (!gc.m_gcLock.m_controlLocked) {
		return;
	}
	gc.m_gcLock.m_controlMutex.unlock();
	gc.m_gcLock.m_controlLocked = false;
}

void interpreter::GarbageCollector::GCLock::GCControlLock()
{
	m_controlMutex.lock();
}

void interpreter::GarbageCollector::GCLock::GCControlRelease()
{
	m_controlMutex.unlock();
}

void interpreter::GarbageCollector::GCLock::GCInstructionsLock()
{
	m_changeInstructionsMutex.lock();
}

void interpreter::GarbageCollector::GCLock::GCInstructionsRelease()
{
	m_changeInstructionsMutex.unlock();
}

void interpreter::GarbageCollector::GCLock::GCBatchLock()
{
	m_batchMutex.lock();
}

void interpreter::GarbageCollector::GCLock::GCBatchRelease()
{
	m_batchMutex.unlock();
}

interpreter::GarbageCollector::GCLock::GCControlLockObject::GCControlLockObject()
{
	GarbageCollector& gc = GetInstance();
	gc.m_gcLock.m_controlMutex.lock();
}

interpreter::GarbageCollector::GCLock::GCControlLockObject::~GCControlLockObject()
{
	GarbageCollector& gc = GetInstance();
	gc.m_gcLock.m_controlMutex.unlock();
}

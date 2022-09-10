#include "garbageCollector.h"

#include <queue>
#include <thread>

namespace
{
	bool m_running = false;
	std::thread* m_thread = nullptr;
	std::vector<interpreter::GarbageCollector::ManagedValue*> m_dead;

	void run()
	{
		while (m_running) {
			interpreter::GarbageCollector::GetInstance().TakeControl();
			interpreter::GarbageCollector::GetInstance().CollectGarbage();
			interpreter::GarbageCollector::GetInstance().ReleaseControl();

			for (int i = 0; i < m_dead.size(); ++i) {
				delete m_dead[i];
			}
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

bool interpreter::GarbageCollector::IsOvercharged()
{
	return m_submitted->size() >= 500;
}

void interpreter::GarbageCollector::CollectGarbage()
{
	std::vector<GCCommand>* m_currentCommands = nullptr;

	m_mutex.lock();

	m_currentCommands = m_submitted;
	if (m_submitted == &m_commands1) {
		m_submitted = &m_commands2;
	}
	else {
		m_submitted = &m_commands1;
	}
	m_submitted->clear();

	m_mutex.unlock();
	
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
	m_dead.clear();

	for (int i = 0; i < m_allValues.size(); ++i) {
		if (m_allValues[i]->m_visited) {
			alive.push_back(m_allValues[i]);
		}
		else {
			m_dead.push_back(m_allValues[i]);
		}
	}

	m_allValues = alive;
}

void interpreter::GarbageCollector::AddExplicitRef(IManagedValue* value)
{
	bool overCharged = IsOvercharged();

	if (overCharged) {
		m_controlMutex.lock();
	}
	m_mutex.lock();

	GCCommand gcCommand;
	gcCommand.m_type = GCCommandType::GCAddExplicitRef;
	gcCommand.value1 = value;
	m_submitted->push_back(gcCommand);

	m_mutex.unlock();
	if (overCharged) {
		m_controlMutex.unlock();
	}
}

void interpreter::GarbageCollector::RemoveExplicitRef(IManagedValue* value)
{
	bool overCharged = IsOvercharged();

	if (overCharged) {
		m_controlMutex.lock();
	}
	m_mutex.lock();

	GCCommand gcCommand;
	gcCommand.m_type = GCCommandType::GCRemoveExplicitRef;
	gcCommand.value1 = value;
	m_submitted->push_back(gcCommand);

	m_mutex.unlock();
	if (overCharged) {
		m_controlMutex.unlock();
	}
}

void interpreter::GarbageCollector::TakeControl()
{
	m_controlMutex.lock();
}

void interpreter::GarbageCollector::ReleaseControl()
{
	m_controlMutex.unlock();
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
	bool overCharged = IsOvercharged();

	if (overCharged) {
		m_controlMutex.lock();
	}
	m_mutex.lock();
	
	GCCommand gcCommand;
	gcCommand.m_type = GCCommandType::GCAddImplicitRef;
	gcCommand.value1 = value;
	gcCommand.value2 = referencedBy;
	m_submitted->push_back(gcCommand);

	m_mutex.unlock();
	if (overCharged) {
		m_controlMutex.unlock();
	}
}

void interpreter::GarbageCollector::RemoveImplicitRef(IManagedValue* value, IManagedValue* referencedBy)
{
	bool overCharged = IsOvercharged();

	if (overCharged) {
		m_controlMutex.lock();
	}
	m_mutex.lock();

	GCCommand gcCommand;
	gcCommand.m_type = GCCommandType::GCRemoveImplicitRef;
	gcCommand.value1 = value;
	gcCommand.value2 = referencedBy;
	m_submitted->push_back(gcCommand);

	m_mutex.unlock();
	if (overCharged) {
		m_controlMutex.unlock();
	}
}

interpreter::IManagedValue::~IManagedValue()
{
}

interpreter::IManagedValue::IManagedValue()
{
}

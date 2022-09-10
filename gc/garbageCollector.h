#pragma once

#include "value.h"

#include <vector>
#include <mutex>

namespace interpreter
{
	class GarbageCollector 
	{
		enum GCCommandType
		{
			None,
			GCAddExplicitRef,
			GCRemoveExplicitRef,
			GCAddImplicitRef,
			GCRemoveImplicitRef,
		};

		struct GCCommand
		{
			GCCommandType m_type = GCCommandType::None;
			IManagedValue* value1 = nullptr;
			IManagedValue* value2 = nullptr;
		};

		struct ManagedValue;

		static GarbageCollector* m_instance;
		GarbageCollector();

		std::vector<ManagedValue*> m_allValues;

		std::vector<GCCommand>* m_submitted = nullptr;
		std::vector<GCCommand> m_commands1;
		std::vector<GCCommand> m_commands2;


		ManagedValue& FindOrCreateValue(IManagedValue* value);
		ManagedValue* FindValue(IManagedValue* value);

		std::mutex m_mutex;
		std::mutex m_controlMutex;

		bool IsOvercharged();
	public:
		struct ManagedValue
		{
			IManagedValue* m_managedValue = nullptr;
			int m_explicitRefs = 0;
			std::vector<IManagedValue*> m_implicitRefs;

			bool m_visited = false;

			~ManagedValue();
		};

		static GarbageCollector& GetInstance();
		static void DisposeGC();

		void CollectGarbage();

		void AddImplicitRef(IManagedValue* value, IManagedValue* referencedBy);
		void RemoveImplicitRef(IManagedValue* value, IManagedValue* referencedBy);

		void AddExplicitRef(IManagedValue* value);
		void RemoveExplicitRef(IManagedValue* value);

		void TakeControl();
		void ReleaseControl();

		~GarbageCollector();
	};
}
#pragma once

#include "value.h"

#include <vector>

namespace interpreter
{
	class GarbageCollector 
	{
		struct ManagedValue
		{
			IManagedValue* m_managedValue = nullptr;
			int m_explicitRefs = 0;
			std::vector<IManagedValue*> m_implicitRefs;

			bool m_visited = false;

			~ManagedValue();
		};

		static GarbageCollector* m_instance;
		GarbageCollector();

		std::vector<ManagedValue*> m_allValues;

		ManagedValue& FindValue(IManagedValue* value);

		void CollectGarbage();

		int m_instructionsBatches = 0;
		bool m_collectingGarbage = false;

	public:

		struct GCInstructionsBatch
		{
			GCInstructionsBatch();
			~GCInstructionsBatch();
		};

		static GarbageCollector& GetInstance();
		static void DisposeGC();

		void AddImplicitRef(IManagedValue* value, IManagedValue* referencedBy);
		void RemoveImplicitRef(IManagedValue* value, IManagedValue* referencedBy);

		void AddExplicitRef(IManagedValue* value);
		void RemoveExplicitRef(IManagedValue* value);

		~GarbageCollector();
	};
}
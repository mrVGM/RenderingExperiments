#pragma once

#include "value.h"

#include <vector>
#include <mutex>
#include <semaphore>

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

		struct ManagedValue
		{
			IManagedValue* m_managedValue = nullptr;
			int m_explicitRefs = 0;
			std::vector<IManagedValue*> m_implicitRefs;

			bool m_visited = false;

			~ManagedValue();
		};

		class GCLock
		{
			std::mutex m_controlMutex;
			std::mutex m_changeInstructionsMutex;
			std::mutex m_batchMutex;

			std::mutex m_exclusiveModeMutex;

			bool m_batchOperation = false;
			bool m_controlLocked = false;
		public:
			struct GCControlLockObject
			{
				GCControlLockObject();
				~GCControlLockObject();
			};

			void AppInstructionsLock();
			void AppInstructionsRelease();
			void AppBatchLock();
			void AppBatchRelease();
			void AppControlLock();
			void AppControlRelease();

			void GCControlLock();
			void GCControlRelease();
			void GCInstructionsLock();
			void GCInstructionsRelease();
			void GCBatchLock();
			void GCBatchRelease();
		};

		static GarbageCollector* m_instance;
		GarbageCollector();

		std::vector<ManagedValue*> m_allValues;

		std::vector<GCCommand>* m_submitted = nullptr;
		std::vector<GCCommand> m_commands1;
		std::vector<GCCommand> m_commands2;

		ManagedValue& FindOrCreateValue(IManagedValue* value);
		ManagedValue* FindValue(IManagedValue* value);

		GCLock m_gcLock;

	public:

		struct GCInstructionsBatch
		{
			GCInstructionsBatch();
			~GCInstructionsBatch();
		};

		static GarbageCollector& GetInstance();
		static void DisposeGC();

		void CollectGarbage();

		void AddImplicitRef(IManagedValue* value, IManagedValue* referencedBy);
		void RemoveImplicitRef(IManagedValue* value, IManagedValue* referencedBy);

		void AddExplicitRef(IManagedValue* value);
		void RemoveExplicitRef(IManagedValue* value);

		~GarbageCollector();
	};
}
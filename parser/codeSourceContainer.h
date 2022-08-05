#pragma once

#include "codeSource.h"
#include "valueResult.h"

#include <string>

namespace scripting
{
	struct CodeSourceContainer
	{
		CodeSourceContainer(const std::string& code, ValueResult& valueResult);
		~CodeSourceContainer();
		CodeSource& GetCodeSource();

		void GetRef();
		void ReleaseRef();
	private:
		void TryDestroy();
		static int m_containersCreated;
		int m_refs = 0;
		CodeSource m_codeSource;
	};
}
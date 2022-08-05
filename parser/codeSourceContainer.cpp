#include "codeSourceContainer.h"

int scripting::CodeSourceContainer::m_containersCreated = 0;

scripting::CodeSourceContainer::CodeSourceContainer(const std::string& code, ValueResult& valueResult)
{
	m_codeSource.m_code = code;
	m_codeSource.Tokenize();
	m_codeSource.TokenizeForParser();

	valueResult.m_codeSourceContainer = this;

	m_refs = 1;
	++m_containersCreated;
}

scripting::CodeSourceContainer::~CodeSourceContainer()
{
	--m_containersCreated;
}

scripting::CodeSource& scripting::CodeSourceContainer::GetCodeSource()
{
	return m_codeSource;
}

void scripting::CodeSourceContainer::GetRef()
{
	++m_refs;
}

void scripting::CodeSourceContainer::ReleaseRef()
{
	--m_refs;
	TryDestroy();
}

void scripting::CodeSourceContainer::TryDestroy()
{
	if (m_refs > 0) {
		return;
	}

	delete this;
}

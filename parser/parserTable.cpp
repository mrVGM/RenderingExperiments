#include "parserTable.h"
#include "dataLib.h"

#include <functional>

bool scripting::ParserTable::Validate()
{
	std::function<bool(const Action&, const Action&)> clash = [](const Action& a, const Action& b) {
		if (a.m_origin != b.m_origin) {
			return false;
		}

		if (a.m_symbol != b.m_symbol) {
			return false;
		}

		return true;
	};

	for (int i = 0; i < m_actions.size() - 1; i++) {
		for (int j = i + 1; j < m_actions.size(); ++j) {
			if (clash(m_actions[i], m_actions[j])) {
				return false;
			}
		}
	}
	return true;
}

void scripting::ParserTable::Serialize(const char* id)
{
	int size = sizeof(int);
	for (int i = 0; i < m_actions.size(); ++i) {
		const Action& action = m_actions[i];
		size += sizeof(bool) + 2 * sizeof(int);
		size += action.m_symbol.size() + 1;
	}

	void* serialized = new char[size];

	memset(serialized, 0, size);

	int* blockSize = reinterpret_cast<int*>(serialized);
	*blockSize = size;

	char* data = reinterpret_cast<char*>(serialized);
	data += sizeof(int);

	for (int i = 0; i < m_actions.size(); ++i) {
		const Action& action = m_actions[i];

		bool* shift = reinterpret_cast<bool*>(data);
		data += sizeof(bool);

		int* origin = reinterpret_cast<int*>(data);
		data += sizeof(int);

		int* target = reinterpret_cast<int*>(data);
		data += sizeof(int);

		*shift = action.m_shift;
		*origin = action.m_origin;
		*target = action.m_target;

		memcpy(data, action.m_symbol.c_str(), action.m_symbol.size());
		data += action.m_symbol.size() + 1;
	}

	data::GetLibrary().WriteBinFile(id, serialized, size);
	delete[] serialized;
}

void scripting::ParserTable::Deserialize(const char* id)
{
	int size;
	data::GetLibrary().ReadBinFile(id, &size, sizeof(int));

	char* serialized = new char[size];

	data::GetLibrary().ReadBinFile(id, serialized, size);
	m_actions.clear();

	size = *reinterpret_cast<int*>(serialized);

	char* data = reinterpret_cast<char*>(serialized);
	char* dataEnd = data + size;

	data += sizeof(int);

	while (data < dataEnd) {
		Action a;
		a.m_shift = *reinterpret_cast<bool*>(data);
		data += sizeof(bool);

		a.m_origin = *reinterpret_cast<int*>(data);
		data += sizeof(int);

		a.m_target = *reinterpret_cast<int*>(data);
		data += sizeof(int);

		a.m_symbol = data;
		data += a.m_symbol.size() + 1;

		m_actions.push_back(a);
	}

	delete[] serialized;
}

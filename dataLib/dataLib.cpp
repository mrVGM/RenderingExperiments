#include "dataLib.h"

#include "json.hpp"

std::string libDir;
data::DataLib* dataLib = nullptr;

data::DataLib::DataLib(const char* dir, const char* name)
{
	libDir = dir;
	FILE* f;
	std::string lib = libDir + name;
	fopen_s(&f, lib.c_str(), "r");
	if (f == nullptr) {
		return;
	}

	char buf[501];
	while (true) {
		memset(buf, 0, 501);
		size_t read = fread(buf, sizeof(char), 500, f);
		m_data += buf;
		if (read < 500) {
			break;
		}
	}
	fclose(f);

	m_json = nlohmann::json::parse(m_data);

	for (auto it = m_json["files"].begin(); it != m_json["files"].end(); ++it) {
		m_fileEntries.push_back(*it);
	}
}

nlohmann::json data::DataLib::GetFileEntry(const char* id) const
{
	return m_json["files"][id];
}

const std::vector<nlohmann::json>& data::DataLib::GetFileEntries() const
{
	return m_fileEntries;
}

std::string data::DataLib::ReadFile(const char* id)
{
	nlohmann::json entry = GetFileEntry(id);
	std::string path = libDir + entry.value("path", "");

	FILE* f;
	fopen_s(&f, path.c_str(), "r");
	if (f == nullptr) {
		return "";
	}

	char buf[501];
	std::string res;
	while (true) {
		memset(buf, 0, 501);
		size_t read = fread(buf, sizeof(char), 500, f);
		res += buf;
		if (read < 500) {
			break;
		}
	}
	fclose(f);

	return res;
}

void data::DataLib::WriteBinFile(const char* id, const void* data, size_t size)
{
	nlohmann::json entry = GetFileEntry(id);
	std::string path = libDir + entry.value("path", "");

	FILE* f;
	fopen_s(&f, path.c_str(), "wb");
	if (f == nullptr) {
		return;
	}
	fwrite(data, 1, size, f);
	fclose(f);
}

size_t data::DataLib::ReadBinFile(const char* id, void* data, size_t size)
{
	nlohmann::json entry = GetFileEntry(id);
	std::string path = libDir + entry.value("path", "");

	FILE* f;
	fopen_s(&f, path.c_str(), "rb");
	if (f == nullptr) {
		return 0;
	}

	char* buf = reinterpret_cast<char*>(data);
	size_t read = 0;
	while (read < size) {
		int tmp = fread(buf + read, 1, size, f);
		read += tmp;
		if (tmp == 0) {
			break;
		}
	}
	fclose(f);

	return read;
}

bool data::Init(const char* dir)
{
	libDir = dir;
	dataLib = new DataLib(dir);
	return dataLib->IsValid();
}

void data::Deinit()
{
	libDir.clear();
	if (dataLib != nullptr) {
		delete dataLib;
	}
	dataLib = nullptr;
}

data::DataLib& data::GetLibrary()
{
	return *dataLib;
}

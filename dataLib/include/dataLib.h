#pragma once

#include <string>
#include "json.hpp"
#include <vector>

namespace data
{
	class DataLib
	{
		std::string m_data;
		nlohmann::json m_json;
		std::vector<nlohmann::json> m_fileEntries;

	public:
		DataLib(const char* dir, const char* name = "lib.json");
		inline bool IsValid() const { return m_data.length() > 0; }
		nlohmann::json GetFileEntry(const char* id) const;
		const std::vector<nlohmann::json>& GetFileEntries () const;

		std::string ReadFile(const char* id);

		void WriteBinFile(const char* id, const void *data, size_t size);
		size_t ReadBinFile(const char* id, void *data, size_t size);
	};

	bool Init(const char* dir);
	void Deinit();

	DataLib& GetLibrary();
}
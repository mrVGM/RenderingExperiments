#include "include/collada.h"

#include "collada.h"
#include "dataLib.h"
#include "parserTable.h"
#include "parser.h"
#include "grammar.h"
#include "codeSource.h"

#include <map>

namespace
{
	collada::IColladaReader* m_reader = nullptr;

	std::string ReadGrammar()
	{
		data::DataLib& dataLib =  data::GetLibrary();

		std::string grammar = dataLib.ReadFileById("collada_grammar");

		return grammar;
	}

	struct ColladaReader : collada::IColladaReader
	{
		struct ColladaFile
		{
			scripting::CodeSource m_codeSource;
			scripting::ISymbol* m_parsed = nullptr;
		};

		std::map<std::string, ColladaFile> m_colladaFiles;

		scripting::Grammar m_grammar;
		scripting::ParserTable m_parserTable;
		scripting::Parser* m_parser = nullptr;

		ColladaReader(const std::string& grammar) :
			m_grammar(grammar)
		{
			m_grammar.GenerateParserStates();
			m_grammar.GenerateParserTable(m_parserTable);

			bool valid = m_parserTable.Validate();

			m_parser = new scripting::Parser(m_grammar, m_parserTable);
		}
		~ColladaReader()
		{
			delete m_parser;
			m_parser = nullptr;
		}

		scripting::ISymbol* ReadColladaFile(const std::string& file) override
		{
			std::map<std::string, ColladaFile>::iterator it = m_colladaFiles.find(file);

			if (it != m_colladaFiles.end()) {
				return it->second.m_parsed;
			}

			data::DataLib& lib = data::GetLibrary();
			std::string fileData = lib.ReadFileByPath(file);

			m_colladaFiles.insert(std::pair<std::string, ColladaFile>(file, ColladaFile()));
			ColladaFile& colladaFile = m_colladaFiles[file];

			colladaFile.m_codeSource.m_code = fileData;
			colladaFile.m_codeSource.Tokenize();
			colladaFile.m_codeSource.TokenizeForColladaReader();

			colladaFile.m_parsed = m_parser->Parse(colladaFile.m_codeSource);

			return colladaFile.m_parsed;
		}
	};
}

collada::IColladaReader* collada::GetReader()
{
	if (!m_reader) {
		std::string grammar = ReadGrammar();
		m_reader = new ColladaReader(grammar);
	}

	return m_reader;
}

void collada::ReleaseColladaReader()
{
	if (m_reader) {
		delete m_reader;
	}

	m_reader = nullptr;
}

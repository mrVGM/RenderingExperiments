#include "tokenizer.h"
#include "codeSource.h"

#include <functional>

bool isBracket(const scripting::ISymbol& symbol)
{
	if (symbol.m_name.size() > 1) {
		return false;
	}

	char c = symbol.m_name[0];
	std::string brackets = "()[]{}";
	int res = brackets.find(c);
	return  res >= 0;
}


std::vector<scripting::ISymbol*> scripting::NewLineSymbolTokenizer::Tokenize(std::vector<ISymbol*>& src)
{
	std::vector<ISymbol*> res;
	ISymbol* carriage = nullptr;
	CodeSource* codeSource = src[0]->m_codeSource;
	for (int i = 0; i < src.size(); ++i) {
		ISymbol& cur = *src[i];
		if (cur.m_name.size() == 1 && cur.m_name[0] == '\r') {
			carriage = &cur;
			continue;
		}

		if (cur.m_name.size() == 1 && cur.m_name[0] == '\n') {
			SimpleSymbol& tmp = *codeSource->CreateSimpleSymbol();
			tmp.m_name = "NewLine";
			tmp.m_codeSource = cur.m_codeSource;
			tmp.m_codePosition = i;

			if (carriage != nullptr) {	
				tmp.m_codePosition = carriage->GetCodePosition();
			}
			res.push_back(&tmp);
			carriage = nullptr;
			continue;
		}

		if (carriage != nullptr) {
			res.push_back(carriage);
		}
		carriage = nullptr;

		res.push_back(&cur);
	}

	if (carriage != nullptr) {
		res.push_back(carriage);
	}

	return res;
}

std::vector<scripting::ISymbol*> scripting::CommentTokenizer::Tokenize(std::vector<ISymbol*>& src)
{
	CodeSource* codeSource = src[0]->m_codeSource;

	std::function<CompositeSymbol* ()> createComment = [&]() {
		CompositeSymbol* res = codeSource->CreateCompositeSymbol();
		res->m_name = "Comment";
		return res;
	};

	std::vector<ISymbol*> res;
	ISymbol* sharp = nullptr;
	CompositeSymbol* comment = nullptr;

	for (int i = 0; i < src.size(); ++i) {
		ISymbol& cur = *src[i];
		if (cur.m_name.size() == 1 && cur.m_name[0] == '#') {
			if (sharp != nullptr) {
				comment->m_childSymbols.push_back(&cur);
				continue;
			}

			sharp = &cur;
			comment = createComment();
			comment->m_childSymbols.clear();
			comment->m_codeSource = cur.m_codeSource;
			comment->m_childSymbols.push_back(&cur);

			continue;
		}

		if (sharp != nullptr) {
			if (cur.m_name == "NewLine") {
				res.push_back(comment);
				res.push_back(&cur);

				sharp = nullptr;
				comment = nullptr;
				continue;
			}
			comment->m_childSymbols.push_back(&cur);
			continue;
		}

		res.push_back(&cur);
	}
	
	if (sharp != nullptr) {
		res.push_back(comment);
	}

	return res;
}

std::vector<scripting::ISymbol*> scripting::StringTokenizer::Tokenize(std::vector<ISymbol*>& src)
{
	CodeSource* codeSource = src[0]->m_codeSource;
	std::vector<ISymbol*> res;

	std::function<CompositeSymbol*()> createStringSymbol = [&] () {
		CompositeSymbol* res = codeSource->CreateCompositeSymbol();
		res->m_name = "String";
		return res;
	};

	CompositeSymbol* string = nullptr;
	std::string symbolData;

	typedef std::function<void(ISymbol&)> handler;

	handler *curHandler;

	handler inString, notInString, escape;

	notInString = [&](ISymbol& next) {
		if (next.m_name.size() == 1 && next.m_name[0] == '"') {
			symbolData = "";
			string = createStringSymbol();
			curHandler = &inString;
			return;
		}

		res.push_back(&next);
	};

	inString = [&](ISymbol& next) {
		if (next.m_name == "\"") {
			string->m_childSymbols.push_back(&next);
			res.push_back(string);
			string->m_symbolData.m_string = symbolData;
			symbolData.clear();
			string = nullptr;
			curHandler = &notInString;
			return;
		}

		if (next.m_name == "NewLine") {
			symbolData += "\n";
			string->m_childSymbols.push_back(&next);
			return;
		}

		if (next.m_name == "\\") {
			curHandler = &escape;
			return;
		}

		symbolData += next.m_name;
		string->m_childSymbols.push_back(&next);
	};

	escape = [&](ISymbol& next) {
		if (next.m_name == "NewLine") {
			symbolData += "\n";
			string->m_childSymbols.push_back(&next);
			curHandler = &inString;
			return;
		}

		symbolData += next.m_name;
		string->m_childSymbols.push_back(&next);
		curHandler = &inString;
	};

	curHandler = &notInString;

	for (int i = 0; i < src.size(); ++i) {
		(*curHandler)(*src[i]);
	}

	m_error = curHandler != &notInString;
	return res;
}

std::vector<scripting::ISymbol*> scripting::BlankTokenizer::Tokenize(std::vector<ISymbol*>& src)
{
	CodeSource* codeSource = src[0]->m_codeSource;

	std::function<CompositeSymbol* ()> createBlank = [&]() {
		CompositeSymbol* res = codeSource->CreateCompositeSymbol();
		res->m_name = "Blank";
		return res;
	};

	std::vector<ISymbol*> res;
	CompositeSymbol* blank = nullptr;

	typedef std::function<void(ISymbol&)> handler;

	handler* curHandler;

	handler inBlank, notInBlank;

	notInBlank = [&](ISymbol& next) {
		if (next.m_name == " " || next.m_name == "\t" || next.m_name == "NewLine") {
			blank = createBlank();
			blank->m_childSymbols.clear();
			blank->m_childSymbols.push_back(&next);
			curHandler = &inBlank;
			return;
		}

		res.push_back(&next);
	};

	inBlank = [&](ISymbol& next) {
		if (next.m_name == " " || next.m_name == "\t" || next.m_name == "NewLine") {
			blank->m_childSymbols.push_back(&next);
			return;
		}

		res.push_back(blank);
		res.push_back(&next);
		blank = nullptr;
		curHandler = &notInBlank;
	};

	curHandler = &notInBlank;

	for (int i = 0; i < src.size(); ++i) {
		(*curHandler)(*src[i]);
	}

	if (curHandler == &inBlank) {
		res.push_back(blank);
	}

	return res;
}


std::vector<scripting::ISymbol*> scripting::NumberTokenizer::Tokenize(std::vector<ISymbol*>& src)
{
	CodeSource* codeSource = src[0]->m_codeSource;

	std::function<CompositeSymbol* ()> createNumber = [&]() {
		CompositeSymbol* name = codeSource->CreateCompositeSymbol();
		name->m_name = "Number";
		return name;
	};

	std::vector<ISymbol*> res;
	CompositeSymbol* number = nullptr;

	typedef std::function<void(ISymbol&)> handler;

	handler* curHandler;

	handler preNumber, notInNumber, signRead, dicimalPart, fractionalPart;

	ISymbol* sign = nullptr;
	double num = 0;
	double fractCoef = 0;
	double coef;

	preNumber = [&](ISymbol& next) {
		coef = 1;
		if (next.m_name.size() == 1 && next.m_name[0] == '-') {
			sign = &next;
			curHandler = &signRead;
			coef = -1;
			return;
		}

		if (next.m_name.size() == 1 && next.m_name[0] >= '0' && next.m_name[0] <= '9') {
			number = createNumber();
			num = next.m_name[0] - '0';
			number->m_childSymbols.push_back(&next);
			curHandler = &dicimalPart;
			return;
		}

		res.push_back(&next);
		if (next.m_name != "String" &&
			next.m_name != "Blank" &&
			!isBracket(next)) {
			curHandler = &notInNumber;
		}
	};

	notInNumber = [&](ISymbol& next) {
		res.push_back(&next);
		if (next.m_name == "String" ||
			next.m_name == "Blank" ||
			isBracket(next)) {
			curHandler = &preNumber;
			return;
		}
	};

	dicimalPart = [&](ISymbol& next) {
		if (next.m_name.size() == 1 && next.m_name[0] >= '0' && next.m_name[0] <= '9') {
			num *= 10;
			num += next.m_name[0] - '0';
			number->m_childSymbols.push_back(&next);
			return;
		}

		if (next.m_name.size() == 1 && next.m_name[0] == '.') {
			number->m_childSymbols.push_back(&next);
			curHandler = &fractionalPart;
			fractCoef = 1;
			return;
		}

		res.push_back(number);
		SymbolData& tmp = res[res.size() - 1]->m_symbolData;
		tmp.m_number = coef * num;
		res.push_back(&next);

		number = nullptr;

		if (next.m_name == "String" ||
			next.m_name == "Blank" ||
			isBracket(next)) {
			curHandler = &preNumber;
			return;
		}

		curHandler = &notInNumber;
	};

	fractionalPart = [&](ISymbol& next) {
		if (next.m_name.size() == 1 && next.m_name[0] >= '0' && next.m_name[0] <= '9') {
			fractCoef /= 10;
			num += fractCoef * (next.m_name[0] - '0');
			number->m_childSymbols.push_back(&next);
			return;
		}

		res.push_back(number);
		SymbolData& tmp = res[res.size() - 1]->m_symbolData;
		tmp.m_number = coef * num;
		res.push_back(&next);
		
		number = nullptr;

		if (next.m_name == "String" ||
			next.m_name == "Blank" ||
			isBracket(next)) {
			curHandler = &preNumber;
			return;
		}

		curHandler = &notInNumber;
	};

	signRead = [&](ISymbol& next) {
		if (next.m_name.size() == 1 && next.m_name[0] >= '0' && next.m_name[0] <= '9') {
			number = createNumber();
			coef = -1;
			num = next.m_name[0] - '0';
			number->m_childSymbols.push_back(sign);
			number->m_childSymbols.push_back(&next);
			
			curHandler = &dicimalPart;
			return;
		}

		coef = 1;
		res.push_back(sign);

		if (next.m_name == "String" ||
			next.m_name == "Blank" ||
			isBracket(next)) {
			curHandler = &preNumber;
			return;
		}
		curHandler = &notInNumber;
	};

	curHandler = &notInNumber;

	for (int i = 0; i < src.size(); ++i) {
		(*curHandler)(*src[i]);
	}

	SimpleSymbol* dummyString = codeSource->CreateSimpleSymbol();
	dummyString->m_name = "String";

	if (curHandler != &notInNumber) {
		(*curHandler)(*dummyString);
	}

	if (res[res.size() - 1] == dummyString) {
		res.erase(res.end() - 1);
	}

	if (curHandler != &notInNumber && curHandler != &preNumber) {
		m_error = true;
	}

	if (res.back()->m_name == "Number") {
		CompositeSymbol* tmp = dynamic_cast<CompositeSymbol*>(res.back());
		if (tmp->m_childSymbols.size() == 0) {
			m_error = true;
		}
		else if (tmp->m_childSymbols.back()->m_name == ".") {
			m_error = true;
		}
	}
	return res;
}

std::vector<scripting::ISymbol*> scripting::NameTokenizer::Tokenize(std::vector<ISymbol*>& src)
{
	CodeSource* codeSource = src[0]->m_codeSource;
	
	std::function<CompositeSymbol*()> createName = [&]() {
		CompositeSymbol* name = codeSource->CreateCompositeSymbol();
		name->m_name = "Name";
		return name;
	};
	
	std::vector<ISymbol*> res;
	typedef std::function<void(ISymbol&)> handler;

	handler* curHandler;

	handler notInName, inName;

	CompositeSymbol* name = nullptr;

	std::function<bool(ISymbol&)> isNameSymbol = [](ISymbol& s) {
		if (s.m_name.size() > 1) {
			return false;
		}
		char c = s.m_name[0];
		
		if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
			return true;
		}

		if (c == '_') {
			return true;
		}

		if (c >= '0' && c <= '9') {
			return true;
		}

		return false;
	};

	std::function<bool(ISymbol&)> isStartNameSymbol = [](ISymbol& s) {
		if (s.m_name.size() > 1) {
			return false;
		}
		char c = s.m_name[0];
		
		if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
			return true;
		}

		if (c == '_') {
			return true;
		}

		return false;
	};

	notInName = [&](ISymbol& next) {
		if (!isStartNameSymbol(next)) {
			res.push_back(&next);
			return;
		}
		name = createName();
		name->m_childSymbols.push_back(&next);
		name->m_symbolData.m_string = next.m_name;
		curHandler = &inName;
	};

	inName = [&](ISymbol& next) {
		if (!isNameSymbol(next)) {
			res.push_back(name);
			res.push_back(&next);
			name = nullptr;
			curHandler = &notInName;
			return;
		}

		name->m_childSymbols.push_back(&next);
		name->m_symbolData.m_string += next.m_name;
	};

	curHandler = &notInName;

	for (int i = 0; i < src.size(); ++i) {
		(*curHandler)(*src[i]);
	}

	if (curHandler == &inName) {
		res.push_back(name);
	}

	m_error = false;
	return res;
}

std::vector<scripting::ISymbol*> scripting::UnsignedNumberTokenizer::Tokenize(std::vector<ISymbol*>& src)
{
	CodeSource* codeSource = src[0]->m_codeSource;

	std::function<CompositeSymbol* ()> createNumber = [&]() {
		CompositeSymbol* name = codeSource->CreateCompositeSymbol();
		name->m_name = "Number";
		return name;
	};

	std::vector<ISymbol*> res;
	CompositeSymbol* number = nullptr;

	typedef std::function<void(ISymbol&)> handler;

	handler* curHandler;

	handler notInNumber, dicimalPart, fractionalPart;

	double num = 0;
	double fractCoef = 0;

	notInNumber = [&](ISymbol& next) {
		if (next.m_name.size() > 1) {
			res.push_back(&next);
			return;
		}

		char c = next.m_name[0];
		if (c < '0' || c > '9') {
			res.push_back(&next);
			return;
		}

		number = createNumber();
		num = next.m_name[0] - '0';
		number->m_childSymbols.push_back(&next);
		curHandler = &dicimalPart;
	};

	dicimalPart = [&](ISymbol& next) {
		if (next.m_name.size() == 1 && next.m_name[0] >= '0' && next.m_name[0] <= '9') {
			num *= 10;
			num += next.m_name[0] - '0';
			number->m_childSymbols.push_back(&next);
			return;
		}

		if (next.m_name.size() == 1 && next.m_name[0] == '.') {
			number->m_childSymbols.push_back(&next);
			curHandler = &fractionalPart;
			fractCoef = 1;
			return;
		}

		res.push_back(number);
		SymbolData& tmp = res[res.size() - 1]->m_symbolData;
		tmp.m_number = num;
		res.push_back(&next);

		number = nullptr;
		curHandler = &notInNumber;
	};

	fractionalPart = [&](ISymbol& next) {
		if (next.m_name.size() == 1 && next.m_name[0] >= '0' && next.m_name[0] <= '9') {
			fractCoef /= 10;
			num += fractCoef * (next.m_name[0] - '0');
			number->m_childSymbols.push_back(&next);
			return;
		}

		res.push_back(number);
		SymbolData& tmp = res[res.size() - 1]->m_symbolData;
		tmp.m_number = num;
		res.push_back(&next);

		number = nullptr;
		curHandler = &notInNumber;
	};

	curHandler = &notInNumber;

	for (int i = 0; i < src.size(); ++i) {
		(*curHandler)(*src[i]);
	}

	SimpleSymbol* dummyString = codeSource->CreateSimpleSymbol();
	dummyString->m_name = "String";

	if (curHandler != &notInNumber) {
		(*curHandler)(*dummyString);
	}

	if (res[res.size() - 1] == dummyString) {
		res.erase(res.end() - 1);
	}

	if (curHandler != &notInNumber) {
		m_error = true;
	}

	if (res.back()->m_name == "Number") {
		CompositeSymbol* tmp = dynamic_cast<CompositeSymbol*>(res.back());
		if (tmp->m_childSymbols.size() == 0) {
			m_error = true;
		}
		else if (tmp->m_childSymbols.back()->m_name == ".") {
			m_error = true;
		}
	}
	return res;
}

scripting::KeywordTokenizer::KeywordTokenizer(const std::string& keyword) :
	m_keyword(keyword)
{
}

std::vector<scripting::ISymbol*> scripting::KeywordTokenizer::Tokenize(std::vector<ISymbol*>& src)
{
	CodeSource* codeSource = src[0]->m_codeSource;

	std::vector<ISymbol*> res;
	
	for (int i = 0; i < src.size(); ++i) {
		if (src[i]->m_name == "Name" && src[i]->m_symbolData.m_string == m_keyword) {
			src[i]->m_name = m_keyword;
		}
		res.push_back(src[i]);
	}

	return res;
}

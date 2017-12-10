
#include "al_FileParser.h"

#include <fstream>
#include "al_Resources.h"


using namespace al;
using namespace std::literals;

std::vector<FileParser::Line> FileParser::open(std::string const& filePath) {
	std::ifstream file{ filePath };
	if (!file.is_open()) {
		throw std::runtime_error{ "The file '" + filePath + "' could not be opened." };
	}
	std::vector<Line> lines;
	std::string line;
	while (getline(file, line)) {
		lines.emplace_back(line);
	}
	return lines;
}

void FileParser::save(std::string const& filePath, std::vector<Line> const& lines) {
	std::ofstream file{ filePath };
	for (auto const& line : lines) {
		file << line.content << '\n';
	}
	//auto lines = open(Resources::folder + ""s + Resources::fileName);
}

FileParser::Line::Line(std::string const& content)
{
	assign(content);
}

FileParser::Line::Line(std::string const& content, LineType type) :
	content{content}, type{type}
{}

void FileParser::Line::assign(std::string const& str) {
	if (str.empty()) {
		type = LineType::Empty;
		content.clear();
		return;
	}
	content = str;
	switch (str[0]) {
	case commentChar:
		type = LineType::Comment;
		break;
	case titleChar:
		type = LineType::Title;
		break;
	default:
		type = LineType::Value;
	}
}

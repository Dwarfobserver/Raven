
#include "al_Resources.h"
#include "al_DataFile.h"
#include <algorithm>


using namespace al;
using namespace std::string_literals;

char const * const Resources::folder = "../../resources/attack-learning/";
char const * const Resources::fileName = "meta.txt";
char const * const Resources::filesTitle = "files";

Resources::Resources() {
	lines_ = FileParser::open(folder + ""s + fileName);
	auto it = lines_.begin();
	while (it->type != FileParser::LineType::Title) ++it;
	// Title
	while (it != lines_.end()) {
		// Files
		if (it->type == FileParser::LineType::Value) {
			files_.emplace_back(it->content);
		}
		++it;
	}
}

Resources::~Resources() noexcept {
	FileParser::save(folder + ""s + fileName, lines_);
}

std::unique_ptr<DataFile> Resources::open(std::string const& fileName) {
	const auto it = find(files_.begin(), files_.end(), fileName);
	if (it == files_.end())
		throw std::runtime_error{ "Tried to open a file not referenced." };

	return std::make_unique<DataFile>(folder + fileName + DataFile::extension);
}

std::unique_ptr<DataFile> Resources::create(std::string const& fileName, Attributes attributes) {
	const auto it = find(files_.begin(), files_.end(), fileName);
	if (it != files_.end())
		throw std::runtime_error{ "Tried to create a file with an already taken name." };
	
	files_.emplace_back(fileName);
	lines_.emplace_back(fileName);
	return std::make_unique<DataFile>(folder + fileName + DataFile::extension, attributes);
}

void Resources::remove(std::string const& fileName) {
	const auto it = find(files_.begin(), files_.end(), fileName);
	if (it == files_.end())
		throw std::runtime_error{ "Tried to remove a file not referenced." };
	files_.erase(it);

	const auto it2 = find_if(lines_.begin(), lines_.end(),
		[&fileName] (FileParser::Line const& val) {
		return val.content == fileName;
	});
	lines_.erase(it2);
}

Resources& al::resources() {
	static Resources instance;
	return instance;
}

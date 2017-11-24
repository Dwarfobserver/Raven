
#include "al_DataFile.h"
#include "al_FileParser.h"
#include "al_Resources.h"


using namespace al;
using namespace std::string_literals;

char const * const DataFile::extension = ".data";
char const * const DataFile::infosTitle = "infos";
char const * const DataFile::contentTitle = "content";

DataFile::DataFile(std::string const& path) :
	path_{ path },
	saveAtDestruction_{false}
{
	lines_ = FileParser::open(path_);
	auto it = lines_.begin();
	while (it->type != FileParser::LineType::Title) ++it;
	// Infos
	while (it->type != FileParser::LineType::Value) ++it;
	// Attributes
	attributes_ = static_cast<Attributes>(std::stoi(it->content));
	while (it->type != FileParser::LineType::Title) ++it;
	// Content
	++it;
	// Data
	while (it != lines_.end()) {
		if (it->type == FileParser::LineType::Value) {
			records_.emplace_back(attributes_, it->content.c_str());
		}
		++it;
	}
}

DataFile::DataFile(std::string const& path, Attributes attributes) :
	path_{ path },
	attributes_{attributes},
	saveAtDestruction_{ true }
{
	lines_.emplace_back(FileParser::titleChar + ""s + infosTitle);
	lines_.emplace_back(std::to_string(intOf(attributes)));
	lines_.emplace_back(FileParser::titleChar + ""s + contentTitle);
}

DataFile::~DataFile() {
	if(saveAtDestruction_)
		save();
}

void DataFile::save() const {
	FileParser::save(path_, lines_);
}

void DataFile::addRecord(Record const& record) {
	lines_.emplace_back(record.serialize());
	records_.push_back(record);
}


#pragma once

#include "al_Attributes.h"
#include "al_FileParser.h"
#include <string>
#include <vector>
#include "al_Record.h"


namespace al {

	class DataFile {
	public:
		static char const * const extension;
		static char const * const infosTitle;
		static char const * const contentTitle;

		// Load existing file
		explicit DataFile(std::string const& path);
		// Create new file
		DataFile(std::string const& path, Attributes attributes);
		// Save
		~DataFile();

		void addRecord(Record const& record);
		void save() const;

		void saveAtDestruction(bool save) { saveAtDestruction_ = save; }
		Attributes attributes() const { return attributes_; }
		std::vector<Record> const& records() const { return records_; }
	private:
		std::string name_;
		std::string path_;
		Attributes attributes_;
		std::vector<FileParser::Line> lines_;
		std::vector<Record> records_;
		bool saveAtDestruction_;
	};

}

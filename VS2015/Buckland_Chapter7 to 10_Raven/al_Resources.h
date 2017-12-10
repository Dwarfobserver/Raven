
#pragma once

#include "al_FileParser.h"
#include "al_DataFile.h"
#include <vector>
#include <memory>


namespace al {

	class Resources
	{
	public:
		static char const * const folder;
		static char const * const fileName;
		static char const * const filesTitle;

		Resources();
		~Resources() noexcept;

		std::unique_ptr<DataFile> open(std::string const& fileName);
		std::unique_ptr<DataFile> create(std::string const& fileName, Attributes attributes);
		void remove(std::string const& fileName);

		std::vector<std::string> const& files() const { return files_; }
	private:
		std::vector<FileParser::Line> lines_;
		std::vector<std::string> files_;
	};

	Resources& resources();
}

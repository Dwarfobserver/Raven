
#pragma once

#include <string>
#include <vector>


namespace al {

	class FileParser {
	public:
		static const char titleChar = '>';
		static const char commentChar = '#';

		struct Line;

		// Read a file and parse it's content
		static std::vector<Line> open(std::string const& filePath);

		static void save(std::string const& filePath, std::vector<Line> const& lines);

		enum class LineType {
			Title,
			Comment,
			Empty,
			Value,
		};
		struct Line {
			explicit Line(std::string const& content);
			Line(std::string const& content, LineType type);

			// Set the line content and deduces it's type
			void assign(std::string const& str);

			std::string content;
			LineType type;
		};
	private:
		std::string path_;
		std::vector<Line> content_;
	};

}

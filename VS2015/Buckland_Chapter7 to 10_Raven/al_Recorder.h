
#pragma once

#include <chrono>
#include <memory>

#include "al_DataFile.h"


class Raven_Bot;

namespace al {

	class Recorder {
	public:
		static char const* FILE_NAME;
		
		static void remove();

		explicit Recorder(Raven_Bot& bot);

		void update();
		void record(bool hasFired);
		Raven_Bot const& bot() const { return bot_; }
	private:
		Raven_Bot& bot_;
		std::unique_ptr<DataFile> pFile_;
		std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdate_;
	};

}

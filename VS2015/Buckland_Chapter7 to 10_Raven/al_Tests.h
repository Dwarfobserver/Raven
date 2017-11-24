
#pragma once

#include <functional>
#include <vector>


namespace al {

	class Tests {
	public:
		static bool run();
	private:
		struct Case {
			Case(std::string&& name, std::function<bool()>&& f);
			std::string name;
			std::function<bool()> test;
			std::string error;
		};
	};

}


#include "al_Record.h"
#include <string>
#include <sstream>


using namespace al;

const float Record::invalidValue = -1.f;

Record::Record() noexcept {
	for (double& d : values_) d = invalidValue;
}

Record::Record(Attributes attributes, std::vector<float> const& values) : Record() {
	int iVec = 0;
	forEachIndice(attributes, [&] (int indice) {
		values_[indice] = values[iVec++];
	});
}

Record::Record(Attributes attributes, char const* line) : Record() {
	std::istringstream ssLine{ line };
	forEachIndice(attributes, [&](int indice) {
		ssLine >> values_[indice];
		char separator;
		ssLine >> separator;
	});
}

Attributes Record::attributes() const {
	auto attr = Attributes::None;
	for (int i = 0; i < 8; ++i) if (values_[i] != invalidValue) {
		attr |= Attributes::FirstFlag << i;
	}
	return attr;
}

std::string Record::serialize() const {
	std::string str;
	for (auto const& f : values_) if (f != invalidValue) {
		str += std::to_string(f) + ';';
	}
	return str;
}

bool al::operator==(Record const& lhs, Record const& rhs) {
	for (int i = 0; i < 8; ++i) {
		if (lhs.values_[i] != rhs.values_[i])
			return false;
	}
	return true;
}

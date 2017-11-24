
#pragma once

#include "al_Attributes.h"
#include <type_traits>


namespace al {

	class Record {
	public:
		static const float invalidValue;

		Record() noexcept;
		Record(Attributes attributes, std::vector<float> const& values);
		Record(Attributes attributes, char const* line);

		Attributes attributes() const;
		std::string serialize() const;

		double& operator[](Attributes attribute)
			{ return values_[indiceOf(attribute)]; }
		double const& operator[](Attributes attribute) const
			{ return values_[indiceOf(attribute)]; }

		double& operator[](int i)
			{ return values_[i]; }
		double const& operator[](int i) const
			{ return values_[i]; }

		friend bool operator==(Record const& lhs, Record const& rhs);
	private:
		double values_[8];
	};

}


#include "al_Attributes.h"


using namespace al;

detail::IndicesArray::IndicesArray() noexcept {
	for (int& i : indices) i = -1;
	int indice = 0;
	int flag = 1;
	while (flag <= intOf(Attributes::LastFlag)) {
		indices[flag] = indice++;
		flag <<= 1;
	}
}

int al::indiceOf(Attributes value) {
	static detail::IndicesArray array{};
	return array.indices[intOf(value)];
}

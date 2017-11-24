
#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>


namespace al {
	
	enum class Attributes : uint8_t {
		None =           0,
		All =			 uint8_t(-1),
		FirstFlag =      1 << 0,
		LastFlag =       1 << 7,

		WeaponType =     1 << 0,
		TargetDistance = 1 << 1,
		AmmoCount =      1 << 2,
		OwnerLife =      1 << 3,
		Attrib5 =        1 << 4,
		Attrib6 =		 1 << 5,
		Attrib7 =	  	 1 << 6,
		Decision =		 1 << 7
	};

	// Get enum type (by value or reference)

	inline uint8_t intOf(Attributes e) {
		return static_cast<uint8_t>(e);
	}
	inline uint8_t& intRefOf(Attributes& e) {
		return *reinterpret_cast<uint8_t*>(&e);
	}

	// Enumeration operators

	inline Attributes operator|(Attributes lhs, Attributes rhs) {
        return static_cast<Attributes>(intOf(lhs) | intOf(rhs));
    }
	inline Attributes operator&(Attributes lhs, Attributes rhs) {
        return static_cast<Attributes>(intOf(lhs) & intOf(rhs));
    }
	inline Attributes operator^(Attributes lhs, Attributes rhs) {
        return static_cast<Attributes>(intOf(lhs) ^ intOf(rhs));
    }

	inline Attributes operator~(Attributes e) {
        return static_cast<Attributes>(~intOf(e));
    }

	inline Attributes& operator|=(Attributes& lhs, Attributes rhs) {
		intRefOf(lhs) |= intOf(rhs);
        return lhs;
    }
	inline Attributes& operator&=(Attributes& lhs, Attributes rhs) {
        intRefOf(lhs) &= intOf(rhs);
		return lhs;
    }
	inline Attributes& operator^=(Attributes& lhs, Attributes rhs) {
        intRefOf(lhs) ^= intOf(rhs);
		return lhs;
    }

	inline Attributes operator<<(Attributes lhs, int shift) {
		return static_cast<Attributes>(intOf(lhs) << shift);
	}
	inline Attributes operator>>(Attributes lhs, int shift) {
		return static_cast<Attributes>(intOf(lhs) >> shift);
	}

	inline Attributes& operator<<=(Attributes& lhs, int shift) {
		intRefOf(lhs) <<= shift;
		return lhs;
	}
	inline Attributes& operator>>=(Attributes& lhs, int shift) {
		intRefOf(lhs) >>= shift;
		return lhs;
	}

	inline bool operator==(Attributes lhs, int rhs) {
		return intOf(lhs) == rhs;
    }
	inline bool operator==(int lhs, Attributes rhs) {
		return lhs == intOf(rhs);
	}
	inline bool operator!=(Attributes lhs, int rhs) {
		return intOf(lhs) != rhs;
	}
	inline bool operator!=(int lhs, Attributes rhs) {
		return lhs != intOf(rhs);
	}

	namespace detail {

		// For compile-time indice

		template <Attributes VALUE, class Expr = void>
		constexpr int indice_of
			= indice_of<VALUE >> 1> +1;

		template <Attributes VALUE>
		constexpr int indice_of<VALUE, std::enable_if_t
			<VALUE & Attributes::FirstFlag>>
			= 0;

		// For static initialisation

		struct IndicesArray {
			IndicesArray() noexcept;
			
			int indices[256];
		};

	}

	int indiceOf(Attributes value);

	template <class F>
	void forEachIndice(Attributes value, F&& f) {
		for (int i = 0; i < 8; ++i) {
			if ((value & Attributes::FirstFlag << i) != 0) f(i);
		}
	}
}

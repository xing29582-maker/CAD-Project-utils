#pragma once

#include "stdint.h"

namespace cadutils
{

	enum class DirtyFlags : uint8_t {
		None = 0,
		Visual = 1 << 0,
		Transform = 1 << 1,
		Geometry = 1 << 2,
	};

	inline DirtyFlags operator|(DirtyFlags a, DirtyFlags b) {
		return static_cast<DirtyFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
	}
	inline bool Has(DirtyFlags a, DirtyFlags b) {
		return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
	}
}
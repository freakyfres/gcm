#pragma once

#include <cstdint>
#include <vector>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

namespace gcm {
	template<typename To, typename From>
	std::vector<To> resizeIntVector(std::vector<From> vec) {
		std::vector<To> ret;
		for (const auto& item : vec) {
			ret.push_back(item);
		}
		return ret;
	}

	constexpr u32 argbToRgba(u32 argb) {
		return (argb >> 24) | ((argb & 0x00FFFFFF) << 8);
	}
} // namespace gcm

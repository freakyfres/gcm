#include "IHasJsObj.hpp"

#include "util.hpp"

#include <napi.h>
#include <print>

namespace gcm {

	template<>
	Napi::Value toJsObject<IHasJsObj>(Napi::Env env, const IHasJsObj& value) {
		return value.toJsObject(env);
	}

	template<>
	Napi::Value toJsObject<std::string>(Napi::Env env,
										const std::string& value) {
		return Napi::String::New(env, value);
	}

	template<>
	Napi::Value toJsObject<bool>(Napi::Env env, const bool& value) {
		return Napi::Boolean::New(env, value);
	}

	template<>
	Napi::Value toJsObject<std::vector<u8>>(Napi::Env env,
											const std::vector<u8>& value) {
		Napi::Uint8Array arr = Napi::Uint8Array::New(env, value.size());
		memcpy(arr.Data(), value.data(), value.size() * sizeof(u8));
		return arr;
	}

	template<>
	Napi::Value toJsObject<u32>(Napi::Env env, const u32& value) {
		return Napi::Number::New(env, value);
	};

	template<>
	Napi::Value toJsObject<std::vector<u32>>(Napi::Env env,
											 const std::vector<u32>& value) {
		auto arr = Napi::Uint32Array::New(env, value.size());
		memcpy(arr.Data(), value.data(), value.size() * sizeof(u32));
		return arr;
	};

	template<>
	Napi::Value toJsObject<std::vector<std::string>>(
		Napi::Env env, const std::vector<std::string>& value) {
		Napi::Array arr = Napi::Array::New(env, value.size());
		for (auto i = 0uz; i < value.size(); i++) {
			arr[i] = toJsObject(env, value[i]);
		}
		return arr;
	};
} // namespace gcm

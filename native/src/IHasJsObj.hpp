#pragma once

#include "napi.h"

namespace gcm {
	template<typename T>
	Napi::Value toJsObject(Napi::Env env, const T& value);
	class IHasJsObj;

	template<>
	Napi::Value toJsObject<IHasJsObj>(Napi::Env env, const IHasJsObj& value);
	template<>
	Napi::Value toJsObject<std::string>(Napi::Env env,
										const std::string& value);
	template<>
	Napi::Value toJsObject<bool>(Napi::Env env, const bool& value);

	template<>
	Napi::Value
	toJsObject<std::vector<uint8_t>>(Napi::Env env,
									 const std::vector<uint8_t>& value);
	template<>
	Napi::Value
	toJsObject<std::vector<std::string>>(Napi::Env env,
										 const std::vector<std::string>& value);

	class IHasJsObj {
	  public:
		IHasJsObj() = default;
		virtual ~IHasJsObj() noexcept = default;
		virtual Napi::Value toJsObject(Napi::Env env) const = 0;
	};
} // namespace gcm

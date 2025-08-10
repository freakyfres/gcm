#include "clipboard.hpp"

#include <cstddef>
#include <napi.h>
#include <thread>

using TSFN = Napi::TypedThreadSafeFunction<std::nullptr_t, void, nullptr>;

static std::thread* clipboardWatcherThread = nullptr;
static std::vector<TSFN> onClipboardChangedCallbacks;

namespace gcm {
	void mkClipboardWatcherThread() {
		if (clipboardWatcherThread != nullptr) {
			throw std::runtime_error("Clipboard watcher thread already exists");
		}
		clipboardWatcherThread = new std::thread([]() {
			clipboard::Watcher watcher;
			while (true) {
				watcher.sleepUntilNextChange();
				for (const auto& callback : onClipboardChangedCallbacks) {
					callback.BlockingCall();
				}
			}
		});
	}

	Napi::Value
	registerClipboardWatcherCallback(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		if (info.Length() != 1 || !info[0].IsFunction()) {
			Napi::TypeError::New(env,
								 "Expected one arugment of a callback function")
				.ThrowAsJavaScriptException();
		}
		TSFN callbackFunction = TSFN::New(
			env, info[0].As<Napi::Function>(), "resource name", 0, 1, nullptr);

		onClipboardChangedCallbacks.push_back(std::move(callbackFunction));
		if (clipboardWatcherThread == nullptr) {
			mkClipboardWatcherThread();
		}

		return env.Undefined();
	}
} // namespace gcm

Napi::Object init(Napi::Env env, Napi::Object exports) {
	exports.Set(
		"registerClipboardWatcherCallback",
		Napi::Function::New(env, gcm::registerClipboardWatcherCallback));
	return exports;
}

NODE_API_MODULE(goodClipboardManagerNative, init);

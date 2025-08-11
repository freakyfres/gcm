#include "clipboard.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <iostream>
#include <optional>
#include <print>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <X11/extensions/Xfixes.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace gcm {
	WindowContext::WindowContext(std::shared_ptr<Display> dpy, Window win):
		display(dpy),
		window(win) {
	}

	std::string WindowContext::getWindowName() const {
		std::string ret;

		Atom actualType;
		int actualFormat;
		ulong nitems;
		ulong bytesAfter;
		u_char* propRet = nullptr;
		if (XGetWindowProperty(display.get(),
							   window,
							   atoms._NET_WM_NAME,
							   0,
							   -1,
							   False,
							   atoms.UTF8_STRING,
							   &actualType,
							   &actualFormat,
							   &nitems,
							   &bytesAfter,
							   &propRet)
			== Success) {
			if (propRet && nitems > 0) {
				ret = std::string((char*)propRet, nitems);
				XFree(propRet);
			}
		} else {
			char* name = nullptr;
			XTextProperty textProp;
			if (XGetWMName(display.get(), window, &textProp)) {
				if (textProp.value && textProp.nitems) {
					ret = std::string((char*)textProp.value, textProp.nitems);
				}
			}
		}
		return ret;
	};

	std::vector<uint8_t> WindowContext::getWindowIcon() const {
		return {};
	};

	std::vector<std::string> WindowContext::getWindowClass() const {
		Atom actualType;
		int actualFormat;
		ulong nitems;
		ulong bytesAfter;
		u_char* propRet = nullptr;
		std::vector<std::string> ret;

		if (XGetWindowProperty(display.get(),
							   window,
							   atoms.WM_CLASS,
							   0,
							   -1,
							   False,
							   XA_STRING,
							   &actualType,
							   &actualFormat,
							   &nitems,
							   &bytesAfter,
							   &propRet)
			== Success) {
			if (propRet && nitems > 0) {
				// WM_CLASS is two null terminated strings
				ret.emplace_back((char*)propRet);
				ret.emplace_back((char*)propRet + strlen((char*)propRet) + 1);
				XFree(propRet);
			}
		}
		return ret;
	};

	std::optional<pid_t> WindowContext::getWindowPid() const {
		std::optional<pid_t> ret {std::nullopt};

		Atom actualType;
		int actualFormat;
		ulong nitems;
		ulong bytesAfter;
		u_char* propRet = nullptr;

		if (XGetWindowProperty(display.get(),
							   window,
							   atoms._NET_WM_PID,
							   0,
							   1,
							   False,
							   XA_CARDINAL,
							   &actualType,
							   &actualFormat,
							   &nitems,
							   &bytesAfter,
							   &propRet)
			== Success) {
			if (propRet && nitems > 0) {
				ret = *(pid_t*)propRet;
				XFree(propRet);
			}
		}
		return ret;
	};

	std::optional<std::string>
	WindowContext::getWindowExePath(pid_t pid) const {
		std::optional<std::string> ret;
		char EXE_PATH[PATH_MAX];
		size_t bytesRead = 0;
		if (bytesRead = readlink(std::format("/proc/{}/exe", pid).c_str(),
								 EXE_PATH,
								 sizeof(EXE_PATH) - 1);
			bytesRead > 0) {
			EXE_PATH[bytesRead] = '\0';
			ret = EXE_PATH;
		}
		return ret;
	};

	WindowInfo WindowContext::getWindowInfo() const {
		WindowInfo ret {};
		ret.pid = getWindowPid();
		ret.iconData = getWindowIcon();
		ret.windowClass = getWindowClass();
		ret.windowTitle = getWindowName();
		ret.exePath = ret.pid ? getWindowExePath(*ret.pid) : std::nullopt;
		return ret;
	}

	std::expected<Window, std::string>
	WindowContext::getActiveWindow(std::shared_ptr<Display> display) {
		Atoms atoms {display};
		Window ret = 0;
		Atom actualType;
		int actualFormat;
		ulong nitems;
		ulong bytesAfter;
		Window* propRet = nullptr;

		if (XGetWindowProperty(display.get(),
							   DefaultRootWindow(display.get()),
							   atoms._NET_ACTIVE_WINDOW,
							   0,
							   -1,
							   False,
							   XA_WINDOW,
							   &actualType,
							   &actualFormat,
							   &nitems,
							   &bytesAfter,
							   (u_char**)&propRet)
			== Success) {
			if (propRet && nitems > 0) {
				ret = *propRet;
				XFree(propRet);
				return ret;
			}
		}
		return std::unexpected("XGetWindowProperty failed");
	}

	void WindowInfo::debug() const {
		std::println("WindowInfo {{");
		std::println("  pid: {}", pid ? std::to_string(*pid) : "No Pid");
		std::println("  hasIconData: {}", iconData.empty() ? "false" : "true");
		std::println("  windowClass: [");
		for (const auto& cls : windowClass) {
			std::println("    `{}`", cls);
		}
		std::println("  ]");
		std::println("  windowTitle: `{}`",
					 windowTitle.empty() ? "No Title" : windowTitle);
		std::println("  exePath: `{}`", exePath ? *exePath : "No Exe Path");
		std::println("}}");
	}

	Napi::Value WindowInfo::toJsObject(Napi::Env env) const {
		auto obj = Napi::Object::New(env);
		obj["pid"] = pid ? Napi::Number::New(env, *pid) : env.Null();
		obj["iconData"] =
			gcm::toJsObject(env, std::vector<uint8_t> {1, 2, 3, 4});
		obj["windowClass"] = gcm::toJsObject(env, windowClass);
		obj["windowTitle"] = gcm::toJsObject(env, windowTitle);
		obj["exePath"] = exePath ? gcm::toJsObject(env, *exePath) : env.Null();
		return obj;
	}

	std::shared_ptr<Display> openDisplay() noexcept {
		Display* dpy = XOpenDisplay(nullptr);

		std::shared_ptr<Display> ret(dpy, [](Display* dpy) noexcept -> void {
			if (dpy) {
				XCloseDisplay(dpy);
			}
		});

		return ret;
	}

	namespace clipboard {

		Napi::Value ClipboardState::toJsObject(Napi::Env env) const {
			auto obj = Napi::Object::New(env);
			obj["changeReason"] = toString(changeReason);
			obj["changeTimestamp"] = Napi::Number::New(env, changeTimestamp);
			obj["appInfo"] = appInfo ? appInfo->toJsObject(env) : env.Null();
			return obj;
		}

		std::string toString(ChangeReason reason) noexcept {
			switch (reason) {
				case ChangeReason::CLOSE:
					return "close";
				case ChangeReason::DESTROY:
					return "destroy";
				case ChangeReason::NEW_OWNER:
					return "new_owner";
				default:
					std::abort();
			}
		}

		Watcher::Watcher():
			display(openDisplay()) {
			atoms = Atoms(display);
			int errorBase;
			XFixesQueryExtension(display.get(), &xfixesEventBase, &errorBase);
			Window root = DefaultRootWindow(display.get());
			XFixesSelectSelectionInput(display.get(),
									   root,
									   atoms.CLIPBOARD,
									   XFixesSetSelectionOwnerNotifyMask);
		}

		Watcher::~Watcher() noexcept {
		}

		void Watcher::sleepUntilNextChange() {
			if (!display) {
				throw std::runtime_error("Display is not initialized");
			}
			XEvent event;
			XNextEvent(display.get(), &event);
			// this is how gtk does it
			// https://gitlab.gnome.org/GNOME/gtk/-/blob/3.24.45/gdk/x11/gdkdisplay-x11.c?ref_type=tags#L1172
			auto selectionEvent = (XFixesSelectionNotifyEvent*)&event;
			if (event.type - xfixesEventBase == XFixesSelectionNotify) {
				writeLatestState(selectionEvent);
			} else {
				writeLatestState(nullptr);
				std::println("?");
			}
		}

		void Watcher::writeLatestState(XFixesSelectionNotifyEvent* event) {
			if (event == nullptr) {
				lastState.reset(nullptr);
				return;
			}
			lastState.reset(new ClipboardState());
			if (event->subtype >= ChangeReason::LAST) {
				std::abort();
			}
			lastState->changeReason = (ChangeReason)event->subtype;
			lastState->changeTimestamp = event->selection_timestamp;

			if (auto window = WindowContext::getActiveWindow(display); window) {
				WindowContext ctx {display, *window};
				lastState->appInfo = ctx.getWindowInfo();
			} else {
				std::cerr << "No active window: " << window.error() << '\n';
			}
		}

		void Watcher::getLatestClipboardState(
			std::unique_ptr<ClipboardState>& state) const {
			if (lastState == nullptr) {
				state = nullptr;
			} else {
				state = std::make_unique<ClipboardState>(*lastState);
			}
		}

	} // namespace clipboard
} // namespace gcm

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

namespace gcm::clipboard {
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

	Watcher::Watcher():
		display(XOpenDisplay(nullptr)) {
		atoms = Atoms(display);
		int errorBase;
		XFixesQueryExtension(display, &xfixesEventBase, &errorBase);
		Window root = DefaultRootWindow(display);
		XFixesSelectSelectionInput(
			display, root, atoms.CLIPBOARD, XFixesSetSelectionOwnerNotifyMask);
	}

	Watcher::~Watcher() noexcept {
		if (display) {
			XCloseDisplay(display);
		}
	}

	void Watcher::sleepUntilNextChange() {
		if (!display) {
			throw std::runtime_error("Display is not initialized");
		}
		XEvent event;
		XNextEvent(display, &event);
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

		if (auto window = getActiveWindow(); window) {
			lastState->appInfo = getWindowInfo(*window);
			lastState->appInfo->debug();
		} else {
			std::cerr << "No active window: " << window.error() << '\n';
		}

		std::println("evetn addr: {:#x}", (uintptr_t)event);
	}

	WindowInfo Watcher::getWindowInfo(Window window) const {
		auto getWindowName = [this](Window window) -> std::string {
			std::string ret;

			Atom actualType;
			int actualFormat;
			ulong nitems;
			ulong bytesAfter;
			u_char* propRet = nullptr;
			if (XGetWindowProperty(display,
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
				if (XGetWMName(display, window, &textProp)) {
					if (textProp.value && textProp.nitems) {
						ret =
							std::string((char*)textProp.value, textProp.nitems);
					}
				}
			}
			return ret;
		};
		auto getWindowIcon = [this](Window window) -> std::vector<char> {
			return {};
		};
		auto getWindowClass =
			[this](Window window) -> std::vector<std::string> {
			Atom actualType;
			int actualFormat;
			ulong nitems;
			ulong bytesAfter;
			u_char* propRet = nullptr;
			std::vector<std::string> ret;

			if (XGetWindowProperty(display,
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
					ret.emplace_back((char*)propRet + strlen((char*)propRet)
									 + 1);
					XFree(propRet);
				}
			}
			return ret;
		};
		auto getWindowPid = [this](Window window) -> std::optional<pid_t> {
			std::optional<pid_t> ret {std::nullopt};

			Atom actualType;
			int actualFormat;
			ulong nitems;
			ulong bytesAfter;
			u_char* propRet = nullptr;

			if (XGetWindowProperty(display,
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
		auto getWindowExePath =
			[this](pid_t pid) -> std::optional<std::string> {
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
		auto pid = getWindowPid(window);
		auto exePath = pid ? getWindowExePath(*pid) : std::nullopt;
		auto ret = WindowInfo {.pid = pid,
							   .iconData = getWindowIcon(window),
							   .windowClass = getWindowClass(window),
							   .windowTitle = getWindowName(window),
							   .exePath = exePath};
		return ret;
	}

	std::expected<Window, std::string> Watcher::getActiveWindow() const {
		Window ret = 0;
		Atom actualType;
		int actualFormat;
		ulong nitems;
		ulong bytesAfter;
		Window* propRet = nullptr;

		if (XGetWindowProperty(display,
							   DefaultRootWindow(display),
							   atoms._NET_ACTIVE_WINDOW,
							   0,
							   -1,
							   False,
							   XA_WINDOW,
							   &actualType,
							   &actualFormat,
							   &nitems,
							   &bytesAfter,
							   (u_char**)&propRet) == Success) {
			if (propRet && nitems > 0) {
				ret = *propRet;
				XFree(propRet);
				return ret;
			}
		}
		return std::unexpected("XGetWindowProperty failed");
	}
} // namespace gcm::clipboard

#include "Atoms.hpp"

namespace gcm {
	Atoms::Atoms(std::shared_ptr<Display> _display) {
    const auto display = _display.get();
		CLIPBOARD = XInternAtom(display, "CLIPBOARD", False);
		_NET_WM_ICON = XInternAtom(display, "_NET_WM_ICON", False);
		_NET_WM_NAME = XInternAtom(display, "_NET_WM_NAME", False);
		WM_NAME = XInternAtom(display, "WM_NAME", False);
		WM_CLASS = XInternAtom(display, "WM_CLASS", False);
		_NET_WM_PID = XInternAtom(display, "_NET_WM_PID", False);
		_NET_ACTIVE_WINDOW = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
		UTF8_STRING = XInternAtom(display, "UTF8_STRING", False);
	}
} // namespace gcm

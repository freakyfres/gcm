#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <cassert>
#include <cstdint>
#include <print>

static Display *dpy;
static Window root;

static int xfixesEventBase;

// ATOMS
static Atom CLIPBOARD;
static Atom WM_CLASS;

void handleEvent(XEvent &evt) {
  if (evt.type - xfixesEventBase == XFixesSelectionNotify) {
    auto evt2 = (XFixesSelectionNotifyEvent &)evt;
    std::println("display ptr: {:#x}", (uintptr_t)evt2.display);
    std::println("window id: {:#x}", evt2.window);
  } else {
    std::println("Unknown event id: {}", evt.type);
  }
}

void watch() {
  std::println("Starting watch");
  std::println("Our window: {:#x}", root);
  std::println("Our display: {:#x}", (uintptr_t)dpy);
  XEvent evt;
  while (true) {
    XNextEvent(dpy, &evt);
    std::println("got event");
    handleEvent(evt);
  }
}

void setupAtoms() {
  assert(dpy && "Display is not set");
  CLIPBOARD = XInternAtom(dpy, "CLIPBOARD", False);
  WM_CLASS = XInternAtom(dpy, "WM_CLASS", False);
  assert(CLIPBOARD && "clipboard atom is not interned");
  assert(WM_CLASS && "WM_CLASS atom is not interned");
}

int main() {
  dpy = XOpenDisplay(nullptr);
  assert(dpy && "Failed to open display");
  setupAtoms();
  root = DefaultRootWindow(dpy);

  XFixesSelectSelectionInput(dpy, root, CLIPBOARD,
                             XFixesSetSelectionOwnerNotifyMask);

  int errorBase;
  XFixesQueryExtension(dpy, &xfixesEventBase, &errorBase);

  watch();
}

#include <X11/Xlib.h>
namespace gcm::clipboard {
class Atoms {
public:
  Atoms() = default;
  Atoms(Display *display);
  ~Atoms() = default;

  Atom CLIPBOARD{None};
  Atom _NET_WM_ICON{None};
  Atom _NET_WM_NAME{None};
  Atom WM_NAME{None};
  Atom WM_CLASS{None};
  Atom _NET_WM_PID{None};
  Atom _NET_ACTIVE_WINDOW{None};
  Atom UTF8_STRING{None};

private:
};
} // namespace gcm::clipboard
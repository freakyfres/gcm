#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>

char* get_window_name(Display *disp, Window win) {
    Atom prop = XInternAtom(disp, "_NET_WM_NAME", False);
    Atom utf8 = XInternAtom(disp, "UTF8_STRING", False);

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop_ret = NULL;

    if (XGetWindowProperty(disp, win, prop, 0, (~0L), False, utf8,
                           &actual_type, &actual_format, &nitems, &bytes_after,
                           &prop_ret) == Success) {
        if (prop_ret) {
            char *name = strndup((char*)prop_ret, nitems);
            XFree(prop_ret);
            return name;
        }
    }
    // fallback to WM_NAME
    char *name = NULL;
    XTextProperty text_prop;
    if (XGetWMName(disp, win, &text_prop)) {
        if (text_prop.value && text_prop.nitems) {
            name = strndup((char*)text_prop.value, text_prop.nitems);
        }
    }
    return name;
}

int main() {
    Display *disp = XOpenDisplay(NULL);
    if (!disp) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }
    
    Atom active = XInternAtom(disp, "_NET_ACTIVE_WINDOW", True);
    if (active == None) {
        fprintf(stderr, "No _NET_ACTIVE_WINDOW atom\n");
        XCloseDisplay(disp);
        return 1;
    }

    Window root = DefaultRootWindow(disp);

    while (1) {
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char *prop_ret = NULL;
        Window active_win = 0;

        if (XGetWindowProperty(disp, root, active, 0, (~0L), False, AnyPropertyType,
                               &actual_type, &actual_format, &nitems, &bytes_after,
                               &prop_ret) == Success) {
            if (prop_ret && nitems) {
                active_win = *(Window *)prop_ret;
                XFree(prop_ret);

                char *name = get_window_name(disp, active_win);
                printf("Active Window: 0x%lx \"%s\"\n", active_win, name ? name : "(no name)");
                free(name);
            } else {
                printf("No active window.\n");
            }
        } else {
            printf("Could not get _NET_ACTIVE_WINDOW property\n");
        }
        fflush(stdout);
        sleep(5);
    }

    XCloseDisplay(disp);
    return 0;
}
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <cstring>

namespace {
Window FindNamedWindow(Display* display, Window root, const char* wanted) {
    char* name = nullptr;
    if (XFetchName(display, root, &name) && name) {
        const bool match = std::strcmp(name, wanted) == 0;
        XFree(name);
        if (match) return root;
    }
    Window root_return = 0, parent = 0, *children = nullptr;
    unsigned int count = 0;
    if (!XQueryTree(display, root, &root_return, &parent, &children, &count)) return 0;
    Window found = 0;
    for (unsigned int i = 0; i < count && !found; ++i)
        found = FindNamedWindow(display, children[i], wanted);
    if (children) XFree(children);
    return found;
}
} // namespace

void MarkWindowUtility(const char* title) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;
    const Window window = FindNamedWindow(display, DefaultRootWindow(display), title);
    if (window) {
        const Atom property = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        const Atom utility = XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
        XChangeProperty(display, window, property, XA_ATOM, 32, PropModeReplace,
                        reinterpret_cast<const unsigned char*>(&utility), 1);
        XFlush(display);
    }
    XCloseDisplay(display);
}

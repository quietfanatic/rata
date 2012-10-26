
#ifdef HEADER

int dbg_frame_number = -1;


#ifndef DISABLE_DEBUGGING
struct Debugger : Named<Debugger> {
    bool active;
    Debugger (CStr name, bool active = false) : Named(name), active(active) { }

    template <class... Args>
    inline void dbg (CStr fmt, Args... args) {
        if (active) {
            fprintf(stderr, "[%d %s] ", dbg_frame_number, name);
            fprintf(stderr, fmt, args...);
        }
    }
};
#else
struct Debugger {
    bool active;
    Debugger (CStr name, bool active) { }
    template <class Args...>
    inline void dbg (CStr fmt, Args... args) { }
};
#endif

#endif


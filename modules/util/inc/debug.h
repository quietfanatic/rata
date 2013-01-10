
#include "registrators.h"

extern uint debug_frame;

struct Debugger : Named<Debugger> {
    std::string name = "";
    bool on = false;
    template <class... Args>
    void dbg (const char* fmt, Args... args) {
        if (on) {
            printf(
            printf(fmt, args...);
        }
    }
    Debugger(const Debugger&) = delete;
};

extern hacc::Map<Debugger> debuggers;

HCB_BEGIN(Debugger) {
    void describe (hacc::Haccer& h, Debugger& it) {
        h.attr("on", it.on);
    }
    Debugger* find_by_haccid (String id) {
        return &debuggers[id];  // Autovivification
    }
    std::string haccid (const Debugger& d) {
         // So dirty!
         // We're assuming that every debugger is a member of a pair, and is preceded by a string in memory.
        typedef std::pair<std::string, Debugger> apair;
        unsigned long offset = (unsigned long)&((apair*)0)->second;
        return ((apair*)((unsigned long)&d - offset))->first;
    }
} HCB_END(Debugger)

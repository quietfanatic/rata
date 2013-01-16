#ifndef HAVE_UTIL_DEBUG_H
#define HAVE_UTIL_DEBUG_H

#include <stdio.h>
#include <string>
#include <unordered_map>

extern int logging_frame;

struct Logger {
    std::string name = "";
    bool on = false;

    static std::unordered_map<std::string, Logger*>& all ();

    void stamp () {
        fprintf(stderr, "[%s %d] ", name.c_str(), logging_frame);
    }
    template <class... Args>
    void log (const char* fmt, Args... args) {
        if (on) {
            stamp();
            fprintf(stderr, fmt, args...);
            fputc('\n', stderr);
        }
    }
    void log (std::string s) {
        if (on) {
            stamp();
            fputs(s.c_str(), stderr);
            fputc('\n', stderr);
        }
    }
    Logger (std::string name, bool on = true) : name(name), on(on) { all().emplace(name, this); }
    Logger (const Logger&) = delete;
};

#endif


#include "../inc/debug.h"
#include "../../hacc/inc/haccable.h"
#include "../../core/inc/commands.h"

namespace util {

    int logging_frame = -1;

    static std::unordered_map<std::string, bool>& log_tags () {
        static std::unordered_map<std::string, bool> r;
        return r;
    }

    static void stamp (const char* tag) {
        fprintf(stderr, "[%s %d] ", tag, logging_frame);
    }
    void log (const char* tag, std::string msg) {
        if (log_tags()[tag]) {
            stamp(tag);
            fprintf(stderr, "%s\n", msg.c_str());
        }
    }
    void log (const char* tag, const char* fmt, ...) {
        if (log_tags()[tag]) {
            stamp(tag);
            va_list args;
            va_start(args, fmt);
            vfprintf(stderr, fmt, args);
            fprintf(stderr, "\n");
            va_end(args);
        }
    }
}

namespace {

void _log (std::string name, bool state) {
    util::log_tags()[name] = state;
}

core::New_Command _log_cmd ("log", "Change whether a particular tag is logged or not.", 2, _log);

}

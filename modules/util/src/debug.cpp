
#include "../inc/debug.h"
#include "../../hacc/inc/haccable.h"
#include "../../core/inc/commands.h"

int logging_frame = -1;


std::unordered_map<std::string, Logger*>& Logger::all () {
    static std::unordered_map<std::string, Logger*> all;
    return all;
}

HACCABLE(Logger*) {
    name("Logger*");
    delegate(value_funcs<std::string>(
        [](Logger* const& l){ return l->name; },
        [](Logger*& l, std::string name){
            auto iter = Logger::all().find(name);
            if (iter != Logger::all().end())
                l = iter->second;
            else
                l = NULL;
        }
    ));
}

namespace {

void _log (std::string name, bool state) {
    auto iter = Logger::all().find(name);
    if (iter != Logger::all().end())
        iter->second->on = state;
}

core::New_Command _log_cmd ("log", "Change whether a particular tag is logged or not.", 2, _log);

}

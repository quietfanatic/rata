
#include "../inc/debug.h"
#include "../../core/inc/commands.h"
#include "../../hacc/inc/haccable.h"

int logging_frame = -1;


std::unordered_map<std::string, Logger*>& Logger::all () {
    static std::unordered_map<std::string, Logger*> all;
    return all;
}

HCB_BEGIN(Logger*)
    name("Logger*");
    delegate(value_funcs<std::string>(
        [](Logger* const& l){ return l->name; },
        [](Logger*& l, std::string name){
            printf("Searchin for logger %s\n", name.c_str());
            auto iter = Logger::all().find(name);
            if (iter != Logger::all().end())
                l = iter->second;
            else
                l = NULL;
        }
    ));
HCB_END(Logger*)

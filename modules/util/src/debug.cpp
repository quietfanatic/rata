
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
    to_tree([](Logger* const& l){ return new hacc::Tree(l->name); });
    delegate(value_functions<std::string>(
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

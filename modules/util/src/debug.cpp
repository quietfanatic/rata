
#include "../inc/debug.h"
#include "../../core/inc/commands.h"
#include "../../hacc/inc/haccable.h"

int logging_frame = -1;


std::unordered_map<std::string, Logger*>& Logger::all () {
    static std::unordered_map<std::string, Logger*> all;
    return all;
}

HCB_BEGIN(Logger*)
    type_name("Logger*");
    to([](Logger* const& l){ return hacc::new_hacc(l->name); });
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

struct Log_Command : Command {
    Logger* logger;
    bool on;
    void operator () () {
        if (logger)
            logger->on = on;
        else core::print_to_console("No such logger found.\n");
    }
};

HCB_BEGIN(Log_Command)
    base<Command>("log");
    command_description<Log_Command>("Enable or disable a certain kind of logging to the terminal.\n");
    elem(member(&Log_Command::logger));
    elem(member(&Log_Command::on));
HCB_END(Log_Command)

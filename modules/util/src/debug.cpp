
#include "../inc/debug.h"
#include "../../core/inc/commands.h"
#include "../../hacc/inc/haccable.h"
#include "../../hacc/inc/haccable_pointers.h"

int logging_frame = -1;


std::unordered_map<std::string, Logger*>& Logger::all () {
    static std::unordered_map<std::string, Logger*> all;
    return all;
}

HCB_BEGIN(Logger)
    type_name("Logger");
    pointee_policy(hacc::REFERENCE);
    get_id([](const Logger& l){ return l.name; });
    find_by_id([](std::string id){
        printf("Searchin for logger %s\n", id.c_str());
        auto iter = Logger::all().find(id);
        if (iter != Logger::all().end())
            return iter->second;
        else
            return (Logger*)NULL;
    });
HCB_END(Logger)

struct Log_Command : Command {
    Logger* logger;
    bool on;
    void operator () () { logger->on = on; }
};

HCB_BEGIN(Log_Command)
    base<Command>("log");
    command_description<Log_Command>("Enable or disable a certain kind of logging to the terminal.\n");
    elem(member(&Log_Command::logger));
    elem(member(&Log_Command::on));
HCB_END(Log_Command)

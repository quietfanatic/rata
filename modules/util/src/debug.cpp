
#include "../inc/debug.h"
#include "../../core/inc/commands.h"
#include "../../hacc/inc/haccable.h"

int logging_frame = -1;


std::unordered_map<std::string, Logger*>& Logger::all () {
    static std::unordered_map<std::string, Logger*> all;
    return all;
}

HCB_BEGIN(Logger)
    get_id([](const Logger& l){ return l.name; });
    find_by_id([](std::string id){
        auto iter = Logger::all().find(id);
        if (iter == Logger::all().end())
            return iter->second;
        else return (Logger*)NULL;
    });
HCB_END(Logger)

struct Log_Command : Command {
    Logger* logger;
    bool on;
    void operator () () { logger->on = on; }
};

HCB_BEGIN(Log_Command)
    base<Command>("log");
    elem(member(&Log_Command::logger));
    elem(member(&Log_Command::on));
HCB_END(Log_Command)


#include "../inc/debug.h"

int logging_frame = -1;


std::unordered_map<std::string, Logger*>& Logger::all () {
    static std::unordered_map<std::string, Logger*> all;
    return all;
}

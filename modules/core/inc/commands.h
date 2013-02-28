#ifndef HAVE_CORE_COMMANDS_H
#define HAVE_CORE_COMMANDS_H

#include <typeinfo>
#include <string>
#include <vector>
#include "../../util/inc/honestly.h"

struct Command {
    virtual void operator() () = 0;
    virtual ~Command () { }
};

namespace core {

    void command_from_string (std::string s);
    void command_from_terminal ();

    void enter_console ();
    void exit_console ();
    void print_to_console (std::string message);
    extern bool console_is_active;
    extern std::string console_contents;
    extern std::string cli_contents;
    extern uint cli_pos;

}

extern std::unordered_map<size_t, std::string> command_descriptions;

template <class Cmd>
void command_description (std::string desc) { command_descriptions.emplace(typeid(Cmd).hash_code(), desc); }

#endif

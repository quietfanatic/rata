#ifndef HAVE_CORE_COMMANDS_H
#define HAVE_CORE_COMMANDS_H

#include <typeinfo>
#include <string>
#include <vector>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

struct Command {
    virtual void operator() () = 0;
    virtual ~Command () { }
};

namespace core {

    void command_from_string (std::string s);
    void command_from_terminal ();

    void console_help ();

    void print_to_console (std::string);

    struct Receives_Output;
    extern Links<Receives_Output> output_receivers;
    struct Receives_Output : Linked<Receives_Output, output_receivers> {
        virtual void receive_output (std::string);
    };

}

 // This is for registering information about commands
extern std::unordered_map<size_t, std::string> command_descriptions;

template <class Cmd>
void command_description (std::string desc) { command_descriptions.emplace(typeid(Cmd).hash_code(), desc); }

#endif

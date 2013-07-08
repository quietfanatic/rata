#ifndef HAVE_CORE_COMMANDS_H
#define HAVE_CORE_COMMANDS_H

#include <typeinfo>
#include <string>
#include <vector>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

struct CommandData;
struct Command : hacc::DPtr<CommandData> {
    void operator () ();
};

namespace core {

    void command_from_string (std::string s);
    void command_from_terminal ();
    extern std::vector<std::string> command_history;

    std::string console_help ();

    void print_to_console (std::string);

    struct Receives_Output;
    extern Links<Receives_Output> output_receivers;
    struct Receives_Output : Linked<Receives_Output, output_receivers> {
        virtual void receive_output (std::string) = 0;
    };

}

struct Command_Description {
    std::string description;
    Type type;
}
extern std::unordered_map<std::string, Command_Description> commands;

template <class Cmd>
void command_description (std::string name, std::string desc) {
    commands.emplace(name, Command_Description{desc, hacc::Type::CppType<Cmd>());
}

#endif

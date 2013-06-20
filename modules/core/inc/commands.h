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
    extern std::vector<std::string> command_history;

    std::string console_help ();

    void print_to_console (std::string);

    struct Receives_Output;
    extern Links<Receives_Output> output_receivers;
    struct Receives_Output : Linked<Receives_Output, output_receivers> {
        virtual void receive_output (std::string) = 0;
    };

}

 // This is for registering information about commands
extern std::unordered_map<size_t, std::string> command_descriptions;

template <class Cmd>
void command_description (std::string desc) { command_descriptions.emplace(typeid(Cmd).hash_code(), desc); }

#define DEF_COMMAND_SP(type, name) type name;
#define DEF_COMMAND_SPD(type, name, def) type name;
#define DEF_COMMAND_HP(type, name) attr(#name, member(&this_type::name));
#define DEF_COMMAND_HPD(type, name, def) attr(#name, member(&this_type::name)(def));
#define DEF_COMMAND(name, desc, params, ...) \
    struct Command_##name : Command { \
        params(DEF_COMMAND_SP, DEF_COMMAND_SPD) \
        void operator () () { __VA_ARGS__ } \
    }; \
    HCB_BEGIN(Command_##name) \
        type_name(STRINGIFY(Command_##name)); \
        command_description(desc); \
        params(DEF_COMMAND_HP, DEF_COMMAND_HPD); \
    HCB_END(Command_##name)

#endif

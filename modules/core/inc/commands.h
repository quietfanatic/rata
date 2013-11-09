#ifndef HAVE_CORE_COMMANDS_H
#define HAVE_CORE_COMMANDS_H

#include <typeinfo>
#include <string>
#include <vector>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

struct CommandData : hacc::DPtee {
    static constexpr bool isa_CommandData = true;
    virtual void operator () () = 0;
};

struct Command final : hacc::DPtr<CommandData> {
    explicit Command (CommandData* d = NULL) : DPtr(d) { }
    void operator () () { (**this)(); }
};

namespace core {
    using namespace util;

    void command_from_string (std::string s);
    void command_from_terminal ();
    extern std::vector<std::string> command_history;

     // TODO: consider whether we really need core::Console
    std::string console_help ();
    void print_to_console (std::string);
    struct Console;
    EXTERN_INIT_SAFE(Links<Console>, consoles);
    struct Console : Linked<Console, consoles> {
        virtual void Console_print (std::string) = 0;
    };

     // Default help message
    std::string no_description_available ();
}

struct Command_Description {
    hacc::Type type;
    std::string name;
    std::string description;
};

EXTERN_INIT_SAFE(std::unordered_map<hacc::TypeData* _COMMA Command_Description*>, commands_by_type);
EXTERN_INIT_SAFE(std::unordered_map<std::string _COMMA Command_Description*>, commands_by_name);

template <class Cmd>
void new_command (std::string name, std::string desc = core::no_description_available()) {
    static_assert(Cmd::isa_CommandData, "isa_CommandData");
    auto cd = new Command_Description{hacc::Type::CppType<Cmd>(), name, desc};
    commands_by_type().emplace(cd->type.data, cd);
    commands_by_name().emplace(cd->name, cd);
}

#endif

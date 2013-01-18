
 // #giveupandusethestl for getline
#include <iostream>
#include <string>
#include <stdexcept>

#include "../../hacc/inc/everything.h"
#include "../inc/commands.h"
#include "../inc/game.h"
#include "../inc/resources.h"

using namespace hacc;

namespace core {

    void command_from_terminal () {
        canonical_ptr<Command> cmd;
        try {
            printf("Command: ");
            std::string cmdline;
            std::getline(std::cin, cmdline);
            if (cmdline.empty()) return;
            hacc::update_from_string(cmd, "[" + cmdline + "]");
        } catch (hacc::Error& e) {
            printf("Error parsing command: %s\n", e.what());
            return;
        } catch (std::exception& e) {
            printf("Error generating command: %s\n", e.what());
            return;
        }
        try {
            (*cmd)();
        } catch (std::exception& e) {
            printf("Error: The command threw an exception: %s\n", e.what());
        }
    }

}

std::unordered_map<size_t, std::string> command_descriptions;

HCB_BEGIN(Command)
    type_name("Command");
HCB_END(Command)

 // Some trivial builtin commands


struct EchoCommand : Command {
    std::string s;
    void operator() () { printf("%s\n", s.c_str()); }
};
HCB_BEGIN(EchoCommand)
    base<Command>("echo");
    elem(member(&EchoCommand::s));
HCB_END(EchoCommand)

struct QuitImmediatelyCommand : Command {
    void operator() () { core::quit_game(); }
};
HCB_BEGIN(QuitImmediatelyCommand)
    base<Command>("quit_immediately");
    empty();
HCB_END(QuitImmediatelyCommand)

struct SeqCommand : Command {
    std::vector<canonical_ptr<Command>> seq;
    void operator() () { for (auto& c : seq) (*c)(); }
};
HCB_BEGIN(SeqCommand)
    base<Command>("seq");
    elem(member(&SeqCommand::seq));
HCB_END(SeqCommand)

 // This will be useful in the future

struct Command_Type : Resource {
    const std::type_info* cpptype;
    void reload () { }
    Command_Type (std::string name) : Resource(name) {
        auto& sublist = Haccable<Command>::get_table()->subtypes;
        auto iter = sublist.find(name);
        if (iter == sublist.end())
            throw std::logic_error("Unknown command.\n");
        cpptype = &iter->second.subtype;
    }
};
static ResourceGroup command_types;
HCB_BEGIN(Command_Type)
    type_name("Command_Type");
    resource_haccability<Command_Type, &command_types>();
HCB_END(Command_Type)

struct HelpCommand : Command {
    Command_Type* ct = NULL;
    void operator() () {
        if (ct == NULL) {
            printf("This is the in-game console.  List of available commands are:\n\n");
            for (auto& sub : Haccable<Command>::get_table()->subtypes) {
                printf("%s ", sub.first.c_str());
            }
            printf("\n\nFor more instructions on a particular command, type 'help <Command>'\n");
        }
        else {
            fputs(ct->name.c_str(), stdout);
            auto& elemlist = HaccTable::require_cpptype(*ct->cpptype)->elems;
            for (auto& e : elemlist) {
                printf(" <%s>", HaccTable::require_cpptype(*e.mtype)->get_type_name().c_str());
                if (e.def.def) printf("?");
            }
            auto iter2 = command_descriptions.find(ct->cpptype->hash_code());
            if (iter2 == command_descriptions.end())
                printf("\nNo information is available about this command.\n");
            else printf("\n%s\n", iter2->second.c_str());
        }
    }
};

HCB_BEGIN(HelpCommand)
    base<Command>("help");
    command_description<HelpCommand>("Show information about the in-game console or about a command");
    elem(member(&HelpCommand::ct, optional<Command_Type*>()));
HCB_END(HelpCommand)



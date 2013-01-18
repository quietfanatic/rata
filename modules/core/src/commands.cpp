
 // #giveupandusethestl for getline
#include <iostream>
#include <string>

#include "../../hacc/inc/everything.h"
#include "../inc/commands.h"
#include "../inc/game.h"

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

struct HelpCommand : Command {
    std::string cmd_name;
    void operator() () {
        if (cmd_name.empty()) {
            printf("This is the in-game console.  List of available commands are:\n\n");
            for (auto& sub : Haccable<Command>::get_table()->subtypes) {
                printf("%s ", sub.first.c_str());
            }
            printf("\n\nFor more instructions on a particular command, type 'help <Command>'\n");
        }
        else {
            auto& sublist = Haccable<Command>::get_table()->subtypes;
            auto iter = sublist.find(cmd_name);
            if (iter == sublist.end()) {
                printf("There is no command named %s\n", cmd_name.c_str());
            }
            else {
                fputs(cmd_name.c_str(), stdout);
                auto& elemlist = HaccTable::require_cpptype(iter->second.subtype)->elems;
                for (auto& e : elemlist) {
                    printf(" <%s>", HaccTable::require_cpptype(*e.mtype)->get_type_name().c_str());
                    if (e.def.def) printf("?");
                }
                auto iter2 = command_descriptions.find(iter->second.subtype.hash_code());
                if (iter2 == command_descriptions.end())
                    printf("\nNo information is available about this command.\n");
                else printf("\n%s\n", iter2->second.c_str());
            }
        }
    }
};

HCB_BEGIN(HelpCommand)
    base<Command>("help");
    command_description<HelpCommand>("Show information about the in-game console or about a command");
    elem(member<std::string>(&HelpCommand::cmd_name, std::string("")));
HCB_END(HelpCommand)




 // #giveupandusethestl for getline
#include <iostream>
#include <string>
#include <stdexcept>

#include "../../hacc/inc/everything.h"
#include "../inc/commands.h"
#include "../inc/input.h"
#include "../inc/game.h"

using namespace hacc;

HCB_BEGIN(Command)
    type_name("Command");
    pointee_policy(FOLLOW);
HCB_END(Command)


namespace core {

    void command_from_string (std::string s) {
        if (s.empty()) return;
        Command* cmd = NULL;
        bool success = false;
        try {
            hacc::update_from_string(cmd, "[" + s + "]");
            success = true;
        } catch (hacc::Error& e) {
            printf("Error parsing command: %s\n", e.what());
        } catch (std::exception& e) {
            printf("Error generating command: %s\n", e.what());
        }
        if (success) {
            try {
                (*cmd)();
            } catch (std::exception& e) {
                printf("Error: The command threw an exception: %s\n", e.what());
            }
        }
        if (cmd) delete cmd;
    }

    void command_from_terminal () {
        printf("Command: ");
        std::string cmdline;
        std::getline(std::cin, cmdline);
        command_from_string(cmdline);
    }

    void console_help () {
        print_to_console("This is the in-game console.  List of available commands are:\n\n");
        for (auto& sub : Haccable<Command>::get_table()->subtypes) {
            print_to_console(sub.first + " ");
        }
        print_to_console("\n\nFor more instructions on a particular command, type 'help <Command>'\n");
    }

    void print_to_console (std::string message) {
        fputs(message.c_str(), stdout);
        for (auto r = output_receivers.first(); r; r = r->next()) {
            r->receive_output(message);
        }
    }

    Links<Receives_Output> output_receivers;


}

using namespace core;

std::unordered_map<size_t, std::string> command_descriptions;

 // Some trivial builtin commands

struct EchoCommand : Command {
    std::string s;
    void operator() () { print_to_console(s + "\n"); }
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
    std::vector<Command*> seq;
    void operator() () { for (auto& c : seq) (*c)(); }
};
HCB_BEGIN(SeqCommand)
    base<Command>("seq");
    elem(member(&SeqCommand::seq));
HCB_END(SeqCommand)

 // This will be useful in the future

struct Command_Type;
std::vector<Command_Type*> command_types;
struct Command_Type : Ordered<Command_Type, command_types> {
    const std::type_info* cpptype;
    Command_Type (std::string name) : Ordered(name) {
        auto& sublist = Haccable<Command>::get_table()->subtypes;
        auto iter = sublist.find(name);
        if (iter == sublist.end())
            throw std::logic_error("Unknown command.\n");
        cpptype = &iter->second.subtype;
    }
};

HCB_BEGIN(Command_Type*)
    type_name("Command_Type*");
    delegate(value_functions<std::string>(
        [](Command_Type* const& ct){
            return ct ? ct->order : std::string("null");
        },
        [](Command_Type*& ctr, std::string s){
            for (auto& ct : command_types) {
                if (ct->order == s) {
                    ctr = ct;
                    return;
                }
            }
            ctr = NULL;
        }
    ));
HCB_END(Command_Type*)

struct HelpCommand : Command {
    Command_Type* ct = NULL;
    void operator() () {
        if (ct == NULL) {
            console_help();
        }
        else {
            print_to_console(ct->order);
            auto& elemlist = HaccTable::require_cpptype(*ct->cpptype)->elems;
            for (auto& e : elemlist) {
                print_to_console(" <" + HaccTable::require_cpptype(*e.mtype)->get_type_name() + ">");
                if (e.def.def) print_to_console("?");
            }
            auto iter2 = command_descriptions.find(ct->cpptype->hash_code());
            if (iter2 == command_descriptions.end())
                print_to_console("\nNo information is available about this command.\n");
            else print_to_console("\n" + iter2->second + "\n");
        }
    }
};

HCB_BEGIN(HelpCommand)
    base<Command>("help");
    command_description<HelpCommand>("Show information about the in-game console or about a command");
    elem(member(&HelpCommand::ct)(optional));
HCB_END(HelpCommand)



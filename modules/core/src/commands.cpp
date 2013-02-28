
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
            print_to_console("Error parsing command: " + std::string(e.what()) + "\n");
        } catch (std::exception& e) {
            print_to_console("Error generating command: " + std::string(e.what()) + "\n");
        }
        if (success) {
            try {
                (*cmd)();
            } catch (std::exception& e) {
                print_to_console("Error: The command threw an exception: " + std::string(e.what()) + "\n");
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

    std::string console_help () {
        std::string r = "This is the in-game console.  List of available commands are:\n\n";
        for (auto& sub : Haccable<Command>::get_table()->subtypes) {
            r += sub.first + " ";
        }
        return r + "\n\nFor more instructions on a particular command, type 'help <Command>'\n";
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
    command_description<EchoCommand>("Print the given string to all console-like places.");
HCB_END(EchoCommand)

struct QuitCommand : Command {
    void operator() () { core::quit_game(); }
};
HCB_BEGIN(QuitCommand)
    base<Command>("quit");
    empty();
    command_description<QuitCommand>("Quit the program without saving anything.");
HCB_END(QuitCommand)

struct SeqCommand : Command {
    std::vector<Command*> seq;
    void operator() () { for (auto& c : seq) (*c)(); }
};
HCB_BEGIN(SeqCommand)
    base<Command>("seq");
    elem(member(&SeqCommand::seq));
    command_description<SeqCommand>("Perform a sequence of commands.  Usage: seq [[command1 args...] [command2 args...]...]");
HCB_END(SeqCommand)


struct HelpCommand : Command {
    std::string type;
    void operator() () {
        if (type.empty()) {
            print_to_console(console_help());
        }
        else {
            auto& sublist = Haccable<Command>::get_table()->subtypes;
            auto iter = sublist.find(type);
            if (iter == sublist.end()) {
                print_to_console("Unknown command " + type + "; available are:");
                for (auto& sub : sublist) {
                    print_to_console(" " + sub.first);
                }
                print_to_console("\n");
            }
            else {
                print_to_console(iter->first);
                auto& elemlist = HaccTable::require_cpptype(iter->second.subtype)->elems;
                for (auto& e : elemlist) {
                    print_to_console(" <" + HaccTable::require_cpptype(*e.mtype)->get_type_name() + ">");
                    if (e.def.def) print_to_console("?");
                }
                auto iter2 = command_descriptions.find(iter->second.subtype.hash_code());
                if (iter2 == command_descriptions.end())
                    print_to_console("\nNo information is available about this command.\n");
                else print_to_console("\n" + iter2->second + "\n");
            }
        }
    }
};

HCB_BEGIN(HelpCommand)
    base<Command>("help");
    command_description<HelpCommand>("Show information about the in-game console or about a command");
    elem(member(&HelpCommand::type)(optional));
HCB_END(HelpCommand)

struct SubtypesCommand : Command {
    std::string supertype;
    void operator () () {
        HaccTable* t = HaccTable::require_type_name(supertype);
        for (auto& sub : t->subtypes) {
            print_to_console(sub.first + " ");
        }
    }
};

HCB_BEGIN(SubtypesCommand)
    base<Command>("subtypes");
    command_description<SubtypesCommand>("Show all registered haccable subtypes of the given type");
    elem(member(&SubtypesCommand::supertype)(required));
HCB_END(SubtypesCommand)


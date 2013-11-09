
 // #giveupandusethestl for getline
#include <string>
#include <stdexcept>
#include <iostream>
#include "../../hacc/inc/everything.h"
#include "../../hacc/src/types_internal.h"
#include "../inc/commands.h"
#include "../inc/input.h"
#include "../inc/window.h"

using namespace hacc;

HCB_BEGIN(Command)
    name("Command");
    to_tree([](const Command& cmd){
        Tree less = Reference(Type(typeid(*cmd)), (void*)&*cmd).to_tree();
        const Array& a =
            less.form() == ARRAY
                ? less.as<const Array&>()
          : less.form() == OBJECT && less.as<const Object&>().size() == 0
                ? Array()
          : throw X::Logic_Error("The CommandData subtype " + Type(typeid(*cmd)).name() + " didn't produce an array tree");
        Array more;
        more.reserve(a.size() + 1);
        auto iter = commands_by_type().find(Type(typeid(*cmd)).data);
        if (iter != commands_by_type().end()) {
            more.emplace_back(Tree(iter->second->name));
            for (auto& t : a)
                more.emplace_back(t);
            return Tree(more);
        }
        else throw X::Logic_Error("A command of type " + Type(typeid(*cmd)).name() + " appeared, but that type wasn't registered as a command");
    });
    prepare([](Command& cmd, Tree tree){
        if (tree.form() != ARRAY)
            throw X::Form_Mismatch(Type::CppType<Command>(), tree);
        auto& a = tree.as<const Array&>();
        if (a.size() == 0)
            throw X::Logic_Error("A command cannot be represented by an empty array");
        if (a[0].form() != STRING)
            throw X::Logic_Error("A command must have a string as its first element");
        std::string name = a[0].as<std::string>();
        auto iter = commands_by_name().find(name);
        if (iter != commands_by_name().end()) {
            auto desc = iter->second;
            void* dat = operator new (desc->type.size());
            desc->type.construct(dat);
            cmd = Command((CommandData*)dat);
            Array less;
            less.reserve(a.size() - 1);
            for (size_t i = 1; i < a.size(); i++)
                less.push_back(a[i]);
            Reference(desc->type, dat).prepare(Tree(std::move(less)));
        }
        else throw X::Logic_Error("No command found named " + name);
    });
    fill([](Command& cmd, Tree tree){
        auto& a = tree.as<const Array&>();
        if (a.size() == 0)
            throw X::Logic_Error("A command cannot be represented by an empty array");
        Array less;
        less.reserve(a.size() - 1);
        for (size_t i = 1; i < a.size(); i++)
            less.push_back(a[i]);
        Reference(
            Type(typeid(*cmd)),
            &*cmd
        ).fill(Tree(std::move(less)));
    });
    finish([](Command& cmd){
        Reference(
            Type(typeid(*cmd)),
            &*cmd
        ).finish();
    });
HCB_END(Command)

namespace core {

    std::vector<std::string> command_history;

    void command_from_string (std::string s) {
        if (s.empty()) return;
        Command cmd;
        bool success = false;
        try {
            Reference(&cmd).from_tree(tree_from_string("[" + s + "]"));
            success = true;
        } catch (X::Error& e) {
            print_to_console("Error parsing command: " + std::string(e.what()) + "\n");
        } catch (std::exception& e) {
            print_to_console("Error generating command: " + std::string(e.what()) + "\n");
        }
        if (success) {
            try {
                if (command_history.empty() || s != command_history.back())
                    command_history.push_back(s);
                cmd();
            } catch (std::exception& e) {
                print_to_console("Error: The command threw an exception: " + std::string(e.what()) + "\n");
            }
        }
    }

    void command_from_terminal () {
        printf("Command: ");
        std::string cmdline;
        std::getline(std::cin, cmdline);
        command_from_string(cmdline);
    }

    std::string console_help () {
        std::string r = "This is the in-game console.  Available commands are:\n\n";
        for (auto& pair : commands_by_name()) {
            r += pair.first + " ";
        }
        return r + "\n\nFor more instructions on a particular command, type 'help <Command>'\n";
    }

    void print_to_console (std::string message) {
        fputs(message.c_str(), stdout);
        for (auto& r : consoles()) {
            r.Console_print(message);
        }
    }

    INIT_SAFE(Links<Console>, consoles);

    std::string no_description_available () {
        static std::string r = "No description is available for this command.";
        return r;
    }

} using namespace core;

INIT_SAFE(std::unordered_map<hacc::TypeData* _COMMA Command_Description*>, commands_by_type);
INIT_SAFE(std::unordered_map<std::string _COMMA Command_Description*>, commands_by_name);

 // Some trivial builtin commands

struct EchoCommand : CommandData {
    std::string s;
    void operator() () { print_to_console(s + "\n"); }
};
HCB_BEGIN(EchoCommand)
    new_command<EchoCommand>("echo", "Print the given string to all console-like places.");
    elem(member(&EchoCommand::s));
HCB_END(EchoCommand)

struct SeqCommand : CommandData {
    std::vector<Command> seq;
    void operator() () { for (auto& c : seq) c(); }
};
HCB_BEGIN(SeqCommand)
    new_command<SeqCommand>("seq", "Perform a sequence of commands.  Usage: seq [[command1 args...] [command2 args...]...]");
    delegate(member(&SeqCommand::seq));
HCB_END(SeqCommand)

struct HelpCommand : CommandData {
    std::string about;
    void operator() () {
        if (about.empty()) {
            print_to_console(console_help());
        }
        else {
            auto iter = commands_by_name().find(about);
            if (iter == commands_by_name().end()) {
                print_to_console("Unknown command " + about + "; available are:");
                for (auto& pair : commands_by_name()) {
                    print_to_console(" " + pair.first);
                }
                print_to_console("\n");
            }
            else {
                Type type = iter->second->type;
                print_to_console(about);
                for (auto& gs : type.data->elem_list) {
                    print_to_console(" <" + gs.type().name() + ">");
                    if (gs->optional) print_to_console("?");
                }
                if (type.data->elems_f || type.data->delegate)
                    print_to_console(" <...>");
                print_to_console("\n" + iter->second->description + "\n");
            }
        }
    }
};

HCB_BEGIN(HelpCommand)
    new_command<HelpCommand>("help", "Show information about the in-game console or about a command");
    elem(member(&HelpCommand::about).optional());
HCB_END(HelpCommand)

struct HistoryCommand : CommandData {
    void operator () () {
        for (uint i = 0; i < command_history.size(); i++) {
            print_to_console(std::to_string(i) + ": " + command_history[i] + "\n");
        }
    }
};

HCB_BEGIN(HistoryCommand)
    new_command<HistoryCommand>("history", "Show previously entered commands");
HCB_END(HistoryCommand)

struct GetCommand : CommandData {
    Reference ref;
    void operator () () {
        print_to_console(hacc::tree_to_string(ref.to_tree(), "", 3) + "\n");
    }
};
HCB_BEGIN(GetCommand)
    new_command<GetCommand>("get", "Print the data at a path");
    elem(member(&GetCommand::ref));
HCB_END(GetCommand)


 // #giveupandusethestl for getline
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include "../../hacc/inc/everything.h"
#include "../../hacc/src/types_internal.h"
#include "../inc/commands.h"
#include "../inc/input.h"
#include "../inc/window.h"

using namespace hacc;
using namespace core;

HACCABLE(Command) {
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
}

 // This is large because we have to manipulate arrays and do dynamically-typed stuff
HACCABLE(Command2) {
    name("Command2");
    to_tree([](const Command2& cmd){
        Tree inner = Reference(Type(typeid(*cmd)), (void*)&*cmd).to_tree();
        auto& less = inner.as<const Array&>();
        Array more;
        more.reserve(less.size() + 1);
        more.push_back(Tree(cmd->info->name));
        for (auto& t : less)
            more.emplace_back(t);
        return Tree(std::move(more));
    });
    prepare([](Command2& cmd, Tree tree){
        auto& more = tree.as<const Array&>();
        if (more.size() == 0)
            throw X::Logic_Error("A command cannot be represented by an empty array");
        if (more[0].form() != STRING)
            throw X::Logic_Error("A command must have a string as its first element");
        std::string name = more[0].as<std::string>();
        auto iter = commands_by_name2().find(name);
        if (iter != commands_by_name2().end()) {
            auto desc = iter->second;
            void* dat = operator new (desc->type.size());
            desc->type.construct(dat);
            cmd = Command2((CommandData2*)dat);
            cmd->info = desc;
            Array less;
            less.reserve(more.size() - 1);
            for (size_t i = 1; i < more.size(); i++)
                less.push_back(more[i]);
            Reference(desc->type, dat).prepare(Tree(std::move(less)));
        }
        else throw X::Logic_Error("No command found named " + name);
    });
    fill([](Command2& cmd, Tree tree){
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
    finish([](Command2& cmd){
        Reference(
            Type(typeid(*cmd)),
            &*cmd
        ).finish();
    });
}

namespace core {

    std::unordered_map<std::string, New_Command*>& commands_by_name2 () {
        static std::unordered_map<std::string, New_Command*> r;
        return r;
    }

    std::vector<std::string> command_history;

    void command_from_string (std::string s) {
        if (s.empty()) return;
        Command2 cmd;
        bool success = false;
        if (command_history.empty() || s != command_history.back())
            command_history.push_back(s);
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
HACCABLE(EchoCommand) {
    new_command<EchoCommand>("echo", "Print the given string to all console-like places.");
    elem(member(&EchoCommand::s));
}

struct SeqCommand : CommandData {
    std::vector<Command> seq;
    void operator() () { for (auto& c : seq) c(); }
};
HACCABLE(SeqCommand) {
    new_command<SeqCommand>("seq", "Perform a sequence of commands.  Usage: seq [[command1 args...] [command2 args...]...]");
    delegate(member(&SeqCommand::seq));
}

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

HACCABLE(HelpCommand) {
    new_command<HelpCommand>("help", "Show information about the in-game console or about a command");
    elem(member(&HelpCommand::about).optional());
}

void _history () {
    for (uint i = 0; i < command_history.size(); i++) {
        print_to_console(std::to_string(i) + ": " + command_history[i] + "\n");
    }
}
New_Command _history_cmd ("history", "Show previously entered commands", 0, _history);

void _get (const Reference& ref) {
    print_to_console(ref.show() + "\n");
    print_to_console(hacc::tree_to_string(ref.to_tree(), "", 3) + "\n");
}
New_Command _get_cmd ("get", "Print the data at a path", 1, _get);

struct KeysCommand : CommandData {
    Reference ref;
    void operator () () {
        auto keys = ref.keys();
        print_to_console(hacc::tree_to_string(Reference(&keys).to_tree()) + "\n");
    }
};
HACCABLE(KeysCommand) {
    new_command<KeysCommand>("keys", "Print the attribute keys of an object");
    elem(member(&KeysCommand::ref));
}

struct LengthCommand : CommandData {
    Reference ref;
    void operator () () {
        print_to_console(std::to_string(ref.length()) + "\n");
    }
};
HACCABLE(LengthCommand) {
    new_command<LengthCommand>("length", "Print the length of an array");
    elem(member(&LengthCommand::ref));
}

struct SetCommand : CommandData {
    Reference ref;
    hacc::Tree value;
    void operator () () {
        ref.from_tree(value);
    }
};
HACCABLE(SetCommand) {
    new_command<SetCommand>("set", "Set the data at a path to a value");
    elem(member(&SetCommand::ref));
    elem(member(&SetCommand::value));
}

struct PeekCommand : CommandData {
    String type;
    size_t address;
    void operator () () {
        Reference ref (Type(type), (void*)address);
        print_to_console(ref.show() + "\n");
        print_to_console(hacc::tree_to_string(ref.to_tree(), "", 3) + "\n");
    }
};
HACCABLE(PeekCommand) {
    new_command<PeekCommand>("peek", "Get the value of a type at an address");
    elem(member(&PeekCommand::type));
    elem(member(&PeekCommand::address));
}

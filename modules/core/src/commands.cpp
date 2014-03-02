#include "core/inc/commands.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "core/inc/window.h"
#include "hacc/inc/everything.h"
#include "util/inc/honestly.h"

using namespace util;
using namespace hacc;

namespace core {

    std::unordered_map<std::string, New_Command*>& commands_by_name () {
        static std::unordered_map<std::string, New_Command*> r;
        return r;
    }

    void Command::operator () () const {
        if (*this) {
            (**this)();
        }
        else {
            throw hacc::X::Logic_Error("Tried to invoke a null Command.\n");
        }
    }

    std::vector<std::string> command_history;

    void command_from_string (std::string s) {
        if (s.empty()) return;
        Command cmd;
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

} using namespace core;

 // Some trivial builtin commands

void _echo (std::string s) { print_to_console(s + "\n"); }
New_Command _echo_cmd ("echo", "Print a string to all console-like places.", 1, _echo);

void _seq (const std::vector<Command>& seq) {
    for (auto& c : seq) c();
}
New_Command _seq_cmd ("seq", "Perform a sequence of commands.  Usage: seq [[command1 args...] [command2 args...]...]", 1, _seq);

void _help (std::string about) {
    if (about.empty()) {
        print_to_console(console_help());
    }
    else {
        auto iter = commands_by_name().find(about);
        if (iter != commands_by_name().end()) {
            print_to_console(about);
            for (size_t i = 0; i < iter->second->arg_types.size(); i++) {
                print_to_console(" <" + iter->second->arg_types[i].name() + ">");
                if (i >= iter->second->min_args) print_to_console("?");
            }
            print_to_console(" : " + iter->second->description + "\n");
        }
        else {
            print_to_console("Unknown command " + about + "; available are:");
            for (auto& pair : commands_by_name()) {
                print_to_console(" " + pair.first);
            }
            print_to_console("\n");
        }
    }
}
New_Command _help_cmd ("help", "Show information about the in-game console or about a command.", 0, _help);

void _history () {
    for (uint i = 0; i < command_history.size(); i++) {
        print_to_console(std::to_string(i) + ": " + command_history[i] + "\n");
    }
}
New_Command _history_cmd ("history", "Show previously entered commands.", 0, _history);

void _load_history (std::string filename) {
    try {
        hacc::Reference(&command_history).from_tree(tree_from_file(filename));
    }
    catch (std::exception& e) {
        print_to_console("History load failed: " + std::string(e.what()) + "\n");
    }
}
New_Command _load_history_cmd ("load_history", "Load command history from a file.", 1, _load_history);
void _save_history (std::string filename) {
    tree_to_file(hacc::Reference(&command_history).to_tree(), filename);
}
New_Command _save_history_cmd ("save_history", "Save command history to a file.", 1, _save_history);

void _get (const Reference& ref) {
    print_to_console(ref.show() + "\n");
    print_to_console(hacc::tree_to_string(ref.to_tree(), "", 3) + "\n");
}
New_Command _get_cmd ("get", "Print the data at a path.", 1, _get);

void __keys (const Reference& ref) {
    auto keys = ref.keys();
    print_to_console(hacc::tree_to_string(Reference(&keys).to_tree()) + "\n");
}
New_Command _keys_cmd ("keys", "Print the attribute keys of an object.", 1, __keys);

void __length (const Reference& ref) {
    print_to_console(std::to_string(ref.length()) + "\n");
}
New_Command _length_cmd ("length", "Print the length of an array.", 1, __length);

void _set (const Reference& ref, hacc::Tree data) {
    ref.from_tree(data);
}
New_Command _set_cmd ("set", "Set the data at a path to a value.", 2, _set);

void _peek (String type, size_t address) {
    Reference ref (Type(type), (void*)address);
    print_to_console(ref.show() + "\n");
    print_to_console(hacc::tree_to_string(ref.to_tree(), "", 3) + "\n");
};
New_Command _peek_cmd ("peek", "Get the value of a type at an address.  Please be careful!", 2, _peek);

struct Copy_Direction {
    bool to;
    bool operator == (Copy_Direction o) { return to == o.to; }
};
HACCABLE(Copy_Direction) {
    value("to", Copy_Direction{true});
    value("from", Copy_Direction{false});
};

void _copy (const Reference& a, Copy_Direction dir, const Reference& b) {
    if (a.type() != b.type()) {
        throw hacc::X::Logic_Error(
            "Arguments to from are of different types "
          + a.type().name() + " and " + b.type().name()
        );
    }
    if (dir.to) {
        a.read([&](void* tp){
            b.write([&](void* fp){
                b.type().copy_assign(b, a);
            });
        });
    }
    else {
        b.read([&](void* tp){
            a.write([&](void* fp){
                a.type().copy_assign(a, b);
            });
        });
    }
}
New_Command _copy_cmd (
    "copy", "Copy data from one location to another.  The second argument is either \"to\" or \"from\".",
    3, _copy
);

 // This is large because we have to manipulate arrays and do dynamically-typed stuff
HACCABLE(Command) {
    name("core::Command");
    to_tree([](const Command& cmd){
        if (!cmd) return Tree(Array());
        Tree inner = Reference(Type(typeid(*cmd)), (void*)&*cmd).to_tree();
        auto& less = inner.as<const Array&>();
        Array more;
        more.reserve(less.size() + 1);
        more.push_back(Tree(cmd->info->name));
        for (auto& t : less)
            more.emplace_back(t);
        return Tree(std::move(more));
    });
    length(value_funcs<size_t>([](const Command& cmd){
        if (!cmd) return (size_t)0;
        return 1 + cmd->info->arg_types.size();
    }, [](Command& cmd, size_t size){
        throw X::Logic_Error("Cannot set length of Command because quietfanatic is a bad hacker.");
    }));
    elems([](Command& cmd, size_t i){
        if (!cmd) throw X::Out_Of_Range(&cmd, i, 0);
        if (i == 0) {
             // Returning a reference to a shared string to keep hacc from
             //  crashing, even though that string must not be modified.
             // This is going to get me into trouble I know it.
            return Reference(&cmd->info->name);
        }
        else {
            return Reference(Type(typeid(*cmd)), (void*)&*cmd).elem(i - 1);
        }
    });
    prepare([](Command& cmd, Tree tree){
        auto& more = tree.as<const Array&>();
        if (more.size() == 0) {
            cmd = Command(null);
            return;
        }
        if (more[0].form() != STRING)
            throw X::Logic_Error("A command must have a string as its first element");
        std::string name = more[0].as<std::string>();
        auto iter = commands_by_name().find(name);
        if (iter != commands_by_name().end()) {
            auto desc = iter->second;
            if (more.size() - 1 < desc->min_args)
                throw X::Logic_Error("Not enough arguments for command " + name);
            void* dat = operator new (desc->type.size());
            desc->type.construct(dat);
            cmd = Command((CommandData*)dat);
            cmd->info = desc;
            Array less;
            less.reserve(more.size() - 1);
            for (size_t i = 1; i < more.size(); i++)
                less.push_back(more[i]);
            Reference(desc->type, dat).prepare(Tree(std::move(less)));
        }
        else throw X::Logic_Error("No command found named " + name);
    });
    fill([](Command& cmd, Tree tree){
        if (!cmd) return;
        auto& a = tree.as<const Array&>();
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
        if (!cmd) return;
        Reference(
            Type(typeid(*cmd)),
            &*cmd
        ).finish();
    });
}

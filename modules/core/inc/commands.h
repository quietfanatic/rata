#ifndef HAVE_CORE_COMMANDS_H
#define HAVE_CORE_COMMANDS_H

#include <typeinfo>
#include <type_traits>
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
     // The API for declaring functions.
    struct New_Command {
        std::string name;
        std::string description;
        size_t min_args;
        hacc::Type type;
        void* func;
        template <class... Args>
        New_Command (std::string, std::string, size_t, void(*)(Args...));
    };
     // And getting them by name.
    std::unordered_map<std::string, New_Command*>& commands_by_name2 ();

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

     // Internal command implementation
    struct CommandData2 : hacc::DPtee {
        New_Command* info;
        virtual void operator () () = 0;
    };
    struct Command2 final : hacc::DPtr<CommandData2> {
        explicit Command2 (CommandData2* d = NULL) : DPtr(d) { }
        void operator () () { (**this)(); }
    };

     // All this is required to unpack a tuple into function arguments.
    template <size_t...> struct _Seq { };
    template <size_t size, size_t... inds>
    struct _Count : _Count<size-1, size-1, inds...> { };
    template <size_t... inds>
    struct _Count<0, inds...> { typedef _Seq<inds...> type; };

    template <class... Args>
    struct CommandDataT : CommandData2 {
        std::tuple<typename std::remove_reference<Args>::type...> args;
        template <size_t... inds>
        void unpack (_Seq<inds...>) {
            (*(void(*)(Args...))info->func)(std::get<inds>(args)...);
        }
        void operator () () {
            unpack(typename _Count<sizeof...(Args)>::type());
        }
    };

    static std::string build_desc (size_t req) { return ""; }
    static std::string build_desc (size_t req, std::string name) {
        return "<" + name + (req > 0 ? ">" : ">?");
    }
    template <class F, class... Args>
    static std::string build_desc (size_t req, F f, Args... names) {
        return "<" + f + (req > 0 ? "> " : ">? ")
             + build_desc(req - 1, names...);
    }

    template <class... Args>
    New_Command::New_Command (
        std::string name, std::string desc, size_t min_args,
        void (* func )(Args... args)
    ) : name(name),
        description(
            name + " " +
            build_desc(min_args, hacc::Type::CppType<std::remove_reference<Args>::type>().name()...)
            + "\n" + desc + "\n"
        ),
        min_args(min_args),
        type(hacc::Type::CppType<CommandDataT<Args...>>()),
        func((void*)func)
    {
        commands_by_name2().emplace(name, this);
    }
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

HACCABLE_TEMPLATE(<class... Args>, core::CommandDataT<Args...>) {
    delegate(member(&core::CommandDataT<Args...>::args));
}

#endif

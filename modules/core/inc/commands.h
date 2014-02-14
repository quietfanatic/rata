#ifndef HAVE_CORE_COMMANDS_H
#define HAVE_CORE_COMMANDS_H

#include <typeinfo>
#include <type_traits>
#include <string>
#include <vector>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"
#include "../../hacc/inc/haccable_standard.h"

namespace core {
     // The API for declaring functions.
    struct New_Command {
        std::string name;
        std::string description;
        uint min_args;
        std::vector<hacc::Type> arg_types;
        hacc::Type type;
        void* func;
        template <class... Args>
        New_Command (std::string, std::string, size_t, void(*)(Args...));
    };
     // And getting them by name.
    std::unordered_map<std::string, New_Command*>& commands_by_name ();

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
    struct CommandData : hacc::DPtee {
        New_Command* info;
        virtual void operator () () const = 0;
    };
    struct Command final : hacc::DPtr<CommandData> {
        explicit Command (CommandData* d = NULL) : DPtr(d) { }
        void operator () () const;
    };

     // All this is required to unpack a tuple into function arguments.
    template <size_t...> struct _Seq { };
    template <size_t size, size_t... inds>
    struct _Count : _Count<size-1, size-1, inds...> { };
    template <size_t... inds>
    struct _Count<0, inds...> { typedef _Seq<inds...> type; };

    template <class... Args>
    struct CommandDataT : CommandData {
        std::tuple<typename std::decay<Args>::type...> args;
        template <size_t... inds>
        void unpack (_Seq<inds...>) const {
            (*(void(*)(Args...))info->func)(std::get<inds>(args)...);
        }
        void operator () () const override {
            unpack(typename _Count<sizeof...(Args)>::type());
        }
    };

    template <class... Args>
    New_Command::New_Command (
        std::string name, std::string desc, size_t min_args,
        void (* func )(Args... args)
    ) : name(name),
        description(desc),
        min_args(min_args),
        arg_types{hacc::Type::CppType<typename std::decay<Args>::type>()...},
        type(hacc::Type::CppType<CommandDataT<Args...>>()),
        func((void*)func)
    {
        commands_by_name().emplace(name, this);
    }
}

HACCABLE_TEMPLATE(<class... Args>, core::CommandDataT<Args...>) {
    delegate(member(&core::CommandDataT<Args...>::args));
}

#endif

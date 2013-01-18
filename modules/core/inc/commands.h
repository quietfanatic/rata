#ifndef HAVE_CORE_COMMANDS_H
#define HAVE_CORE_COMMANDS_H

#include <typeinfo>

struct Command {
    virtual void operator() () = 0;
    virtual ~Command () { }
};

namespace core {

    void command_from_terminal ();

}

extern std::unordered_map<size_t, std::string> command_descriptions;

template <class Cmd>
void command_description (std::string desc) { command_descriptions.emplace(typeid(Cmd).hash_code(), desc); }

#endif

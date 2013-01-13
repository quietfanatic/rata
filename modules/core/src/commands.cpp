
 // #giveupandusethestl for getline
#include <iostream>
#include <string>

#include "../../hacc/inc/everything.h"
#include "../inc/commands.h"

namespace core {

    void command_from_terminal () {
        printf("Command: ");
        std::string cmdline;
        std::getline(std::cin, cmdline);
        Command* cmd = hacc::value_from_string<hacc::follow_ptr<Command>>("[" + cmdline + "]");
        (*cmd)();
    }

}

 // Some trivial builtin commands

struct EchoCommand : Command {
    std::string s;
    void operator() () { printf("%s\n", s.c_str()); }
};

HCB_BEGIN(EchoCommand)
    base<Command>("echo");
    elem(member(&EchoCommand::s));
HCB_END(EchoCommand)

//struct QuitImmediatelyCommand {
//    operator () { quit_game(); }
//};

//HCB_BEGIN(QuitImmediatelyCommand)
//    empty();
//HCB_END(QuitImmediatelyCommand)



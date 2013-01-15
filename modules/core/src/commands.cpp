
 // #giveupandusethestl for getline
#include <iostream>
#include <string>

#include "../../hacc/inc/everything.h"
#include "../inc/commands.h"
#include "../inc/game.h"

using namespace hacc;

namespace core {

    void command_from_terminal () {
        printf("Command: ");
        std::string cmdline;
        std::getline(std::cin, cmdline);
        canonical_ptr<Command> cmd;
        hacc::update_from_string(cmd, "[" + cmdline + "]");
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


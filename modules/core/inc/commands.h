#ifndef HAVE_CORE_COMMANDS_H
#define HAVE_CORE_COMMANDS_H

struct Command {
    virtual void operator() () = 0;
    virtual ~Command () { }
};

namespace core {

    void command_from_terminal ();

}

#endif

#ifndef HAVE_CORE_CONSOLE_H
#define HAVE_CORE_CONSOLE_H

struct Command {
    virtual void operator() () = 0;
    virtual ~Command () { }
};

namespace core {

    void command_from_terminal ();

}

#endif


#include <stdlib.h>
#include "../inc/phases.h"
#include "../inc/commands.h"
#include "../../hacc/inc/everything.h"

namespace core {

    INIT_SAFE(std::vector<Phase*>, all_phases)

} using namespace core;

HCB_BEGIN(Phase*)
    name("core::Phase*");
    hacc::hacc_pointer_by_member(&Phase::name, all_phases());
HCB_END(Phase*)

struct Allow_Command : CommandData {
    Phase* phase = NULL;
    void operator () () {
        if (phase)
            phase->on = true;
        else {
            print_to_console("Available phases are:\n");
            for (auto p : all_phases()) {
                std::string name = p->name.empty() ? "<anonymous>" : p->name;
                print_to_console("\t" + name + " \"" + p->order + "\" " + (p->on ? "true" : "false") + "\n");
            }
        }
    }
};
HCB_BEGIN(Allow_Command)
    new_command<Allow_Command>("allow", "Turn a Phase on (shows available Phases with no argument)");
    elem(member(&Allow_Command::phase).optional());
HCB_END(Allow_Command)

struct Disallow_Command : CommandData {
    Phase* phase;
    void operator () () {
        if (phase) phase->on = false;
    }
};
HCB_BEGIN(Disallow_Command)
    new_command<Disallow_Command>("disallow", "Turn a Phase off");
    elem(member(&Disallow_Command::phase));
HCB_END(Disallow_Command)

